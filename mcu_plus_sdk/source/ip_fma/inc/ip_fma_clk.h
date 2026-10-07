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
 *  \defgroup IP_FMA_CLK DCC module Register Check Wrappers
 */

/**
 *  \ingroup  IP_FMA_CLK
 *  \defgroup IP_FMA_CLK_INTERFACE DCC module Register Check Interface
 *
 *  @{
 */

/**
 *  \file     ip_fma_clk.h
 *
 *  \brief    CLK-specific wrappers over IP_FMA Register Check.
 *
 */

#ifndef IP_FMA_CLK_H_
#define IP_FMA_CLK_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ip_fma_common.h"
#include <sdl/dcc/v0/soc/sdl_dcc_soc.h>

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief DCC register snapshot/expected set for DCC.
 */
typedef struct
{
    uint32_t dcc_gctrl;                  /* DCC Global Control Register */
    uint32_t dcc_cntseed0;               /* Count0 Seed Value Register */
    uint32_t dcc_validseed0;             /* Valid0 Seed Value Register */
    uint32_t dcc_cntseed1;               /* Count1 Seed Value Register */
    uint32_t dcc_clksrc1;                /* Clock Source Selection Register 1 */
    uint32_t dcc_clksrc0;                /* Clock Source Selection Register 0 */
    uint32_t dcc_gctrl2;                 /* DCC Global Control Register 2 */
} IpFma_DccRegs;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Read DCC registers into \c out.
 *
 * \param instance Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Clk_GetDccRegs(SDL_DCC_Inst instance, IpFma_DccRegs* out);

/**
 * \brief Compare DCC registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Clk_CompareDccRegs(const IpFma_DccRegs* expected,
                                         const IpFma_DccRegs* actual);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_CLK_H_ */

/** @} */
