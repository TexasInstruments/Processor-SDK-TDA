/*
 *  Copyright (C) 2026 Texas Instruments Incorporated
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
 *  \brief Entry point for the DSS "Software Read-back of Written Configuration"
 *         safety-mechanism example.
 *
 *  The application body is reused unchanged from the dss_display_test example
 *  (dss_display_test_main()); this file only provides the entry point and the
 *  read-back error callback.
 *
 *  This example links the drivers_ipfma library (the drivers library built with
 *  the DSS readack enabled) instead of the standard drivers library. After  DSS
 *  configuration register is written, csl_dssCommon.c re-reads it and, on a
 *  mismatch, calls CSL_DSSReadbackErr(). The CSL declares this callback but provides
 *  no default, so this file must define it - a read-back-enabled build that omits it
 *  fails to link.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <stdint.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/dss.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void dss_display_test_main(void *args);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Read-back error callback for the DSS "Software Read-back of Written
 * Configuration" safety mechanism. csl_dssCommon.c, compiled into the
 * drivers_ipfma library with the read-back enabled, calls this on a write/read-back mismatch
 * and provides no default, so this example must define it (the build fails to
 * link otherwise).
 */
void CSL_DSSReadbackErr(uint32_t expVal, uint32_t readVal)
{
    DebugP_log("Call of the read-back error callback function \r\n");
    DebugP_log("DSS read-back: register MISMATCH. Expected: 0x%08X, Read: 0x%08X\r\n",
               expVal, readVal);
}

int main()
{
    int32_t status = SystemP_SUCCESS;

    System_init();
    Board_init();

    /* Open drivers */
    Drivers_open();
    /* Open board component drivers */
    status = Board_driversOpen();
    DebugP_assert(status==SystemP_SUCCESS);

    dss_display_test_main(NULL);

    /* Close boards memory drivers */
    Board_driversClose();
    /* Close drivers */
    Drivers_close();

    Board_deinit();
    System_deinit();

    return 0;
}
