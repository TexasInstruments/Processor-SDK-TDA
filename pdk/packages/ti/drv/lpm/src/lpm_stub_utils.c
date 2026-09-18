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
 * \file lpm_stub_utils.c
 *
 * \brief Utility functions shared across the LPM stub and its supporting
 *        modules. Only compiled as part of the lpm_stub baremetal library.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/csl/soc.h>
#include <ti/drv/lpm/src/lpm_stub_utils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_stubCtrlMmrUnlock(uint32_t base, uint32_t partition)
{
    CSL_REG32_WR_OFF(base,
                     LPM_STUB_CTRL_MMR_LOCKn_KICK0_OFFSET +
                     ((partition * LPM_STUB_CTRL_MMR_PARTITION_STRIDE) + LPM_STUB_CTRL_MMR_LOCK_CTRL_OFFSET),
                     LPM_STUB_CTRL_MMR_KICK0_UNLOCK_VAL);
    CSL_REG32_WR_OFF(base,
                     LPM_STUB_CTRL_MMR_LOCKn_KICK1_OFFSET +
                     ((partition * LPM_STUB_CTRL_MMR_PARTITION_STRIDE) + LPM_STUB_CTRL_MMR_LOCK_CTRL_OFFSET),
                     LPM_STUB_CTRL_MMR_KICK1_UNLOCK_VAL);
}

void Lpm_stubHangAbort(void)
{
    volatile uint32_t hang = LPM_STUB_HANG_INIT_VAL;
    while (hang != 0U)
    {
        /* Spin in a loop in case of failure because we cannot abort from this stage */
    }
}
