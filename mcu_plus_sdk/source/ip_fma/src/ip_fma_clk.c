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
 *  \ingroup  IP_FMA_CLK
 *  \defgroup IP_FMA_CLK_IMPLEMENTATION DCC module Register Check Implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_clk.c
 *
 *  \brief    DCC-specific wrappers over IP_FMA Register Check.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "ip_fma_clk.h"
#include <sdl/dcc/v0/sdlr_dcc2.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Clk_GetDccRegs(SDL_DCC_Inst instance, IpFma_DccRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        uint32_t baseAddr;
        int32_t sdlStatus = SDL_EFAIL;
        sdlStatus = SDL_DCC_getBaseaddr(instance, &baseAddr);

        if (SDL_PASS != sdlStatus)
        {
            status = IPFMA_E_PARAM;
        }
        else
        {
            IpFma_RegDesc registerDescriptorsArr[] =
            {
                { SDL_DCC2_DCCGCTRL,            0u, IPFMA_WIDTH_32 },
                { SDL_DCC2_DCCCNTSEED0,         0u, IPFMA_WIDTH_32 },
                { SDL_DCC2_DCCVALIDSEED0,       0u, IPFMA_WIDTH_32 },
                { SDL_DCC2_DCCCNTSEED1,         0u, IPFMA_WIDTH_32 },
                { SDL_DCC2_DCCCLKSRC1,          0u, IPFMA_WIDTH_32 },
                { SDL_DCC2_DCCCLKSRC0,          0u, IPFMA_WIDTH_32 },
                { SDL_DCC2_DCCGCTRL2,           0u, IPFMA_WIDTH_32 },
            };

            status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr,
                        (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

            if (IPFMA_OK == status)
            {
                out->dcc_gctrl        = registerDescriptorsArr[0].value;
                out->dcc_cntseed0     = registerDescriptorsArr[1].value;
                out->dcc_validseed0   = registerDescriptorsArr[2].value;
                out->dcc_cntseed1     = registerDescriptorsArr[3].value;
                out->dcc_clksrc1      = registerDescriptorsArr[4].value;
                out->dcc_clksrc0      = registerDescriptorsArr[5].value;
                out->dcc_gctrl2       = registerDescriptorsArr[6].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Clk_CompareDccRegs(const IpFma_DccRegs* expected,
                                         const IpFma_DccRegs* actual)
{
    IpFma_Status status = IPFMA_OK;    

    if (((NULL_PTR == expected)) || ((NULL_PTR == actual)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        /* DCCENA bits [3:0] change during single-shot operation (0x0 after
         * auto-completion, 0xF while enabled). Mask them out and only compare
         * the static configuration fields (ERRENA, SINGLESHOT, DONEENA). */
        if (((expected->dcc_gctrl & ~SDL_DCC2_DCCGCTRL_DCCENA_MASK) !=
             (actual->dcc_gctrl   & ~SDL_DCC2_DCCGCTRL_DCCENA_MASK)))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dcc_cntseed0 != actual->dcc_cntseed0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dcc_validseed0 != actual->dcc_validseed0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dcc_cntseed1 != actual->dcc_cntseed1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dcc_clksrc1 != actual->dcc_clksrc1))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dcc_clksrc0 != actual->dcc_clksrc0))
        {
            status = IPFMA_E_MISMATCH;
        }
        if ((IPFMA_OK == status) && (expected->dcc_gctrl2 != actual->dcc_gctrl2))
        {
            status = IPFMA_E_MISMATCH;
        }       
    }

    return status;
}

/** @} */
