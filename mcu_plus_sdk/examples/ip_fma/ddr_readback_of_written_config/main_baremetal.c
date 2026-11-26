/*
 *  Copyright (C) 2023-26 Texas Instruments Incorporated
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

/*
 *  \brief DDR Software Read-back of Written Configuration safety-mechanism demo.
 *
 *  Built as a minimal bootloader so it runs before DDR is brought up. In that
 *  context DDR_init() (invoked from System_init() via the DDR SysConfig module)
 *  actually applies the controller/PHY/PI register configuration - because
 *  DDR_isEnabled() is false - and every write is read back and verified by the
 *  read-back-enabled LPDDR4 driver from the drivers_ipfma library this example links.
 *  The read-back runs before the console UART is open, so the
 *  LPDDR4_ddrReadbackErr() callback only records; main() reports the result once
 *  the UART is up, then idles (this bootloader boots nothing).
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <kernel/dpl/DebugP.h>
#include <drivers/bootloader.h>

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* DDR write-config read-back results, recorded by the callback during DDR_init()
 * (which runs in System_init(), before the console UART is open) and reported
 * from main() once the UART is up. */
static volatile uint32_t gDdrWrRbMismatchCount = 0U;
static volatile uint32_t gDdrWrRbFirstExp      = 0U;
static volatile uint32_t gDdrWrRbFirstRead     = 0U;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* Strong override of the weak LPDDR4_ddrReadbackErr() in the DDR controller
 * driver. Called via RB_CHECK_REG32 whenever a just-written DDR config register
 * reads back wrong. Records only (runs before the console UART is open). */
void LPDDR4_ddrReadbackErr(uint32_t expVal, uint32_t readVal)
{
    if (gDdrWrRbMismatchCount == 0U)
    {
        gDdrWrRbFirstExp  = expVal;
        gDdrWrRbFirstRead = readVal;
    }
    gDdrWrRbMismatchCount++;
}

int main(void)
{
    int32_t status;

    Bootloader_socWaitForFWBoot();

    /* DDR is brought up here: System_init() -> DDR_init(). DDR_isEnabled() is
     * false (first boot stage), so DDR_initHwRegs() applies the config and the
     * write-config read-back fires on every CTL/PHY/PI register. */
    System_init();

    status = Bootloader_socOpenFirewalls();
    DebugP_assertNoLog(status == SystemP_SUCCESS);

    Board_init();
    Drivers_open();

    DebugP_log("\r\nDDR Software Read-back of Written Configuration\r\n");
    DebugP_log("DDR write-config read-back: %u mismatch(es) during DDR init\r\n",
               (uint32_t)gDdrWrRbMismatchCount);
    if (gDdrWrRbMismatchCount == 0U)
    {
        DebugP_log("All DDR config registers verified. All tests have passed!\r\n");
    }
    else
    {
        DebugP_log("  first mismatch: expected 0x%08X, read 0x%08X\r\n",
                   (uint32_t)gDdrWrRbFirstExp, (uint32_t)gDdrWrRbFirstRead);
        DebugP_log("Some tests have failed\r\n");
    }

    Drivers_close();

    Board_deinit();
    System_deinit();

    return 0;
}
