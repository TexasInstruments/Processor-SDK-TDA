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
 *  \file     main_baremetal.c
 *
 *  \brief    This example application demonstrates periodic software readback
 *            of static CSIRX D-PHY and SHIM configuration registers and
 *            reports match or mismatch between expected and actual values for
 *            validation and diagnostic purposes.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ========================================================================== */
/*                 External Function Declarations                             */
/* ========================================================================== */

extern void CsirxApp_moduleClockEnable(void);
extern void CsirxApp_moduleClockDisable(void);
extern void CsirxApp_PeriodicReadback(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    int32_t status = SystemP_SUCCESS;

    System_init();
    Board_init();

    /* Enable clock on all CSI RX domains */
    (void)CsirxApp_moduleClockEnable();

    Drivers_open();
    status = Board_driversOpen();
    DebugP_assert(status == SystemP_SUCCESS);

    DebugP_log("Calling CsirxApp_PeriodicReadback()\r\n");

    (void)CsirxApp_PeriodicReadback();

    Board_driversClose();
    Drivers_close();

    Board_deinit();

    /* Disable clock on all CSI RX domains */
    (void)CsirxApp_moduleClockDisable();

    System_deinit();

    return 0;
}

/********************************* End of file ******************************/
