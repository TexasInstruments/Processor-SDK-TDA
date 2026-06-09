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
 *  \ingroup  IP_FMA_PSC
 *  \defgroup IP_FMA_PSC_IMPLEMENTATION PSC Register Check Implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_psc.c
 *
 *  \brief    PSC-specific wrappers over IP_FMA Register Check
 *            for readback of static configuration registers.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "ip_fma_psc.h"
#include <ti/csl/cslr_psc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Psc_GetRegs(uintptr_t baseAddr, IpFma_PscRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            { CSL_PSC_PTCMD(0x0U),      0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PTCMD(0x1U),      0u, IPFMA_WIDTH_32 },
            
            { CSL_PSC_PDCTL(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x01U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x02U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x03U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x04U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x05U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x06U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x07U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x08U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x09U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x0AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x0BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x0CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x0DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x0EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x0FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x10U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x11U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x12U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x13U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x14U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x15U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x16U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x17U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x18U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x19U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x1AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x1BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x1CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x1DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x1EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x1FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x20U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x21U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x22U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x23U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x24U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x25U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x26U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x27U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x28U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x29U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x2AU),     0u, IPFMA_WIDTH_32 },

            { CSL_PSC_PDCFG(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x01U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x02U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x03U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x04U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x05U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x06U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x07U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x08U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x09U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x0AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x0BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x0CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x0DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x0EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x0FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x10U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x11U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x12U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x13U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x14U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x15U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x16U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x17U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x18U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x19U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x1AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x1BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x1CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x1DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x1EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x1FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x20U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x21U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x22U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x23U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x24U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x25U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x26U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x27U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x28U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x29U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x2AU),     0u, IPFMA_WIDTH_32 },          

            { CSL_PSC_MDCFG(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x01U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x02U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x03U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x04U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x05U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x06U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x07U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x08U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x09U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x10U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x11U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x12U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x13U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x14U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x15U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x16U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x17U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x18U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x19U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x20U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x21U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x22U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x23U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x24U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x25U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x26U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x27U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x28U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x29U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x2AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x2BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x2CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x2DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x2EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x2FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x30U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x31U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x32U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x33U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x34U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x35U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x36U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x37U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x38U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x39U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x3AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x3BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x3CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x3DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x3EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x3FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x40U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x41U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x42U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x43U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x44U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x45U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x46U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x47U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x48U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x49U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x4AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x4BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x4CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x4DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x4EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x4FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x50U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x51U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x52U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x53U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x54U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x55U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x56U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x57U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x58U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x59U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x5AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x5BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x5CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x5DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x5EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x5FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x60U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x61U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x62U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x63U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x64U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x65U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x66U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x67U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x68U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x69U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x6AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x6BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x6CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x6DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x6EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x6FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x70U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x71U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x72U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x73U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x74U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x75U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x76U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x77U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x78U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x79U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x7AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x7BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x7CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x7DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x7EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x7FU),     0u, IPFMA_WIDTH_32 },

            { CSL_PSC_MDCTL(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x01U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x02U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x03U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x04U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x05U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x06U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x07U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x08U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x09U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x10U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x11U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x12U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x13U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x14U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x15U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x16U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x17U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x18U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x19U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x20U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x21U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x22U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x23U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x24U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x25U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x26U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x27U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x28U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x29U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x2AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x2BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x2CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x2DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x2EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x2FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x30U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x31U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x32U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x33U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x34U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x35U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x36U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x37U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x38U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x39U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x3AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x3BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x3CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x3DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x3EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x3FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x40U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x41U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x42U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x43U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x44U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x45U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x46U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x47U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x48U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x49U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x4AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x4BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x4CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x4DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x4EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x4FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x50U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x51U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x52U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x53U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x54U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x55U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x56U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x57U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x58U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x59U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x5AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x5BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x5CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x5DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x5EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x5FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x60U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x61U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x62U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x63U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x64U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x65U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x66U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x67U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x68U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x69U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x6AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x6BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x6CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x6DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x6EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x6FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x70U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x71U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x72U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x73U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x74U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x75U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x76U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x77U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x78U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x79U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x7AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x7BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x7CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x7DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x7EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x7FU),     0u, IPFMA_WIDTH_32 },
        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            uint32_t idx = 0;
            
            for (uint32_t i = 0; i < PSC_PTCMD_GR_NUM; i++)
            {
                out->psc_ptcmd[i] = registerDescriptorsArr[idx++].value;
            }
            
            for (uint32_t i = 0; i < PSC_PD_NUM; i++)
            {
                out->psc_pdctl[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0; i < PSC_PD_NUM; i++)
            {
                out->psc_pdcfg[i] = registerDescriptorsArr[idx++].value;
            }
           
            for (uint32_t i = 0; i < PSC_MD_NUM; i++)
            {
                out->psc_mdcfg[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0; i < PSC_MD_NUM; i++)
            {
                out->psc_mdctl[i] = registerDescriptorsArr[idx++].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Psc_CompareRegs(const IpFma_PscRegs* expected,
                                    const IpFma_PscRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((expected == NULL) || (actual == NULL))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        for (uint32_t i = 0; i < PSC_PTCMD_GR_NUM; i++)
        {
            if (expected->psc_ptcmd[i] != actual->psc_ptcmd[i])
            {
                status = IPFMA_E_MISMATCH;
                break;
            }
        }

        if (IPFMA_OK == status)
        {
            for (uint32_t i = 0; i < PSC_PD_NUM; i++)
            {
                if ((expected->psc_pdctl[i] != actual->psc_pdctl[i]) ||
                    (expected->psc_pdcfg[i] != actual->psc_pdcfg[i]))
                {
                    status = IPFMA_E_MISMATCH;
                    break;
                }
            }
        }

        if (IPFMA_OK == status)
        {
            for (uint32_t i = 0; i < PSC_MD_NUM; i++)
            {
                if ((expected->psc_mdcfg[i] != actual->psc_mdcfg[i]) ||
                    (expected->psc_mdctl[i] != actual->psc_mdctl[i]))
                {
                    status = IPFMA_E_MISMATCH;
                    break;
                }
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Psc_GetMcuRegs(uintptr_t baseAddr, IpFma_PscMcuRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            { CSL_PSC_PTCMD(0x0U),      0u, IPFMA_WIDTH_32 },
            
            { CSL_PSC_PDCTL(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x01U),     0u, IPFMA_WIDTH_32 },

            { CSL_PSC_PDCFG(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x01U),     0u, IPFMA_WIDTH_32 },

            { CSL_PSC_MDCFG(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x01U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x02U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x03U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x04U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x05U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x06U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x07U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x08U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x09U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x10U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x11U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x12U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x13U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x14U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x15U),     0u, IPFMA_WIDTH_32 },

            { CSL_PSC_MDCTL(0x00U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x01U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x02U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x03U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x04U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x05U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x06U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x07U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x08U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x09U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0AU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0BU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0CU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0DU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0EU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0FU),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x10U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x11U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x12U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x13U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x14U),     0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x15U),     0u, IPFMA_WIDTH_32 },
        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            uint32_t idx = 0;

            out->psc_mcu_ptcmd = registerDescriptorsArr[idx++].value;            

            for (uint32_t i = 0; i < PSC_MCU_PD_NUM; i++)
            {
                out->psc_mcu_pdctl[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0; i < PSC_MCU_PD_NUM; i++)
            {
                out->psc_mcu_pdcfg[i] = registerDescriptorsArr[idx++].value;
            }
           
            for (uint32_t i = 0; i < PSC_MCU_MD_NUM; i++)
            {
                out->psc_mcu_mdcfg[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0; i < PSC_MCU_MD_NUM; i++)
            {
                out->psc_mcu_mdctl[i] = registerDescriptorsArr[idx++].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Psc_CompareMcuRegs(const IpFma_PscMcuRegs* expected,
                                      const IpFma_PscMcuRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((expected == NULL) || (actual == NULL))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->psc_mcu_ptcmd != actual->psc_mcu_ptcmd)
        {
            status = IPFMA_E_MISMATCH;
        }

        if (IPFMA_OK == status)
        {
            for (uint32_t i = 0; i < PSC_MCU_PD_NUM; i++)
            {
                if ((expected->psc_mcu_pdctl[i] != actual->psc_mcu_pdctl[i]) ||
                    (expected->psc_mcu_pdcfg[i] != actual->psc_mcu_pdcfg[i]))
                {
                    status = IPFMA_E_MISMATCH;
                    break;
                }
            }

            if (IPFMA_OK == status)
            {
                for (uint32_t i = 0; i < PSC_MCU_MD_NUM; i++)
                {
                    if ((expected->psc_mcu_mdcfg[i] != actual->psc_mcu_mdcfg[i]) ||
                        (expected->psc_mcu_mdctl[i] != actual->psc_mcu_mdctl[i]))
                    {
                        status = IPFMA_E_MISMATCH;
                        break;
                    }
                }
            }
        }
    }

    return status;
}

#if defined(SOC_J784S4)
IpFma_Status IpFma_Psc_GetBoltonRegs(uintptr_t baseAddr, IpFma_PscBoltonRegs* out)
{
    IpFma_Status status = IPFMA_OK;

    if (NULL_PTR == out)
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        static IpFma_RegDesc registerDescriptorsArr[] =
        {
            { CSL_PSC_PTCMD(0x0U),  0u, IPFMA_WIDTH_32 },

            { CSL_PSC_PDCTL(0x00U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x01U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x02U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x03U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x04U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x05U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x06U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCTL(0x07U), 0u, IPFMA_WIDTH_32 },

            { CSL_PSC_PDCFG(0x00U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x01U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x02U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x03U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x04U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x05U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x06U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_PDCFG(0x07U), 0u, IPFMA_WIDTH_32 },

            { CSL_PSC_MDCFG(0x00U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x01U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x02U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x03U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x04U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x05U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x06U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x07U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x08U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x09U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0AU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0BU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0CU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0DU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0EU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x0FU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x10U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x11U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x12U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x13U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x14U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x15U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x16U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x17U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x18U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x19U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCFG(0x1AU), 0u, IPFMA_WIDTH_32 },

            { CSL_PSC_MDCTL(0x00U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x01U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x02U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x03U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x04U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x05U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x06U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x07U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x08U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x09U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0AU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0BU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0CU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0DU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0EU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x0FU), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x10U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x11U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x12U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x13U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x14U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x15U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x16U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x17U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x18U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x19U), 0u, IPFMA_WIDTH_32 },
            { CSL_PSC_MDCTL(0x1AU), 0u, IPFMA_WIDTH_32 },            
        };

        status = IpFma_GetRegsValues(baseAddr, registerDescriptorsArr, (uint32_t)(sizeof(registerDescriptorsArr) / sizeof(registerDescriptorsArr[0])));

        if (IPFMA_OK == status)
        {
            uint32_t idx = 0U;

            out->psc_bolton_ptcmd = registerDescriptorsArr[idx++].value;

            for (uint32_t i = 0U; i < PSC_BOLTON_PD_NUM; i++)
            {
                out->psc_bolton_pdctl[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0U; i < PSC_BOLTON_PD_NUM; i++)
            {
                out->psc_bolton_pdcfg[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0U; i < PSC_BOLTON_MD_NUM; i++)
            {
                out->psc_bolton_mdcfg[i] = registerDescriptorsArr[idx++].value;
            }

            for (uint32_t i = 0U; i < PSC_BOLTON_MD_NUM; i++)
            {
                out->psc_bolton_mdctl[i] = registerDescriptorsArr[idx++].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Psc_CompareBoltonRegs(const IpFma_PscBoltonRegs* expected,
                                         const IpFma_PscBoltonRegs* actual)
{
    IpFma_Status status = IPFMA_OK;

    if ((expected == NULL) || (actual == NULL))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        if (expected->psc_bolton_ptcmd != actual->psc_bolton_ptcmd)
        {
            status = IPFMA_E_MISMATCH;
        }

        if (IPFMA_OK == status)
        {
            for (uint32_t i = 0; i < PSC_BOLTON_PD_NUM; i++)
            {
                if ((expected->psc_bolton_pdctl[i] != actual->psc_bolton_pdctl[i]) ||
                    (expected->psc_bolton_pdcfg[i] != actual->psc_bolton_pdcfg[i]))
                {
                    status = IPFMA_E_MISMATCH;
                    break;
                }
            }

            if (IPFMA_OK == status)
            {
                for (uint32_t i = 0; i < PSC_BOLTON_MD_NUM; i++)
                {
                    if ((expected->psc_bolton_mdcfg[i] != actual->psc_bolton_mdcfg[i]) ||
                        (expected->psc_bolton_mdctl[i] != actual->psc_bolton_mdctl[i]))
                    {
                        status = IPFMA_E_MISMATCH;
                        break;
                    }
                }
            }
        }
    }

    return status;
}

#endif

/** @} */
