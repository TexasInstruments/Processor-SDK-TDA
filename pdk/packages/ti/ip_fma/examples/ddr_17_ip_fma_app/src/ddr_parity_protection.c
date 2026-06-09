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
 *  \ingroup  DDR_PARITY_PROTECTION
 *  \defgroup DDR_PARITY_PROTECTION_IMPLEMENTATION DDR parity protection configuration API implementation
 *
 *  @{
 */

/**
 *  \file     ddr_parity_protection.c
 *
 *  \brief    DDR parity protection configuration API implementation.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ddr_parity_protection.h>
#include <ti/csl/csl_types.h>

/* ========================================================================== */
/*                           File Scope Variables                             */
/* ========================================================================== */

/*	==========================================================================	*/
/*	                          Macros & Typedefs                               	*/
/*	==========================================================================	*/

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void IpFma_Ddr17_EnableCtlParityProtection(uint32_t baseAddress)
{
    /* Enable parity protection and injection*/
    uint32_t* ctlParityConfigReg1 = (uint32_t*) (baseAddress + DDRSS_CTL_PCFG1_REG_CTL_442_OFFSET);
    *ctlParityConfigReg1 |= DDRSS_CTL_PCFG1_REGPORT_ADDR_PARITY_PROTECTION_EN;
    *ctlParityConfigReg1 |= DDRSS_CTL_PCFG1_REGPORT_WRITEMASK_PARITY_PROTECTION_EN;
    *ctlParityConfigReg1 |= DDRSS_CTL_PCFG1_REGPORT_WRITE_PARITY_PROTECTION_EN;

    uint32_t* ctlParityConfigReg2 = (uint32_t*) (baseAddress + DDRSS_CTL_PCFG1_REG_CTL_443_OFFSET);
    *ctlParityConfigReg2 |= DDRSS_CTL_PCFG2_REGPORT_READ_PARITY_PROTECTION_EN;
    *ctlParityConfigReg2 |= DDRSS_CTL_PCFG2_PARAMREG_PARITY_PROTECTION_EN;
}

int32_t IpFma_Ddr17_EnableCtlParityProtectionInjection(uint32_t baseAddress, IpFma_Ddr17CtlParityErrorSource source)
{
    IpFma_Status status = IPFMA_OK;

    uint32_t* ctlParityConfigReg2 = (uint32_t*) (baseAddress + DDRSS_CTL_PCFG1_REG_CTL_443_OFFSET);
    uint32_t* ctlParityConfigReg3 = (uint32_t*) (baseAddress + DDRSS_CTL_PCFG3_REG_CTL_444_OFFSET);

    switch (source)
    {
        case ADDRESS_PARITY_ERROR:
            *ctlParityConfigReg2 |= DDRSS_CTL_PCFG2_REGPORT_ADDR_PARITY_PROTECTION_INJECTION_EN;
            break;

        case WRITE_DATA_MASK_PARITY_ERROR:
            *ctlParityConfigReg2 |= DDRSS_CTL_PCFG2_REGPORT_WRITEMASK_PARITY_PROTECTION_INJECTION_EN;
            break;

        case PARAM_REGISTER_PARITY_ERROR:
            *ctlParityConfigReg3 |= DDRSS_CTL_PCFG3_PARAMREG_PARITY_PROTECTION_INJECTION_EN;
            break;

        case READ_DATA_PARITY_ERROR:
            *ctlParityConfigReg3 |= DDRSS_CTL_PCFG3_REGPORT_READ_PARITY_PROTECTION_INJECTION_EN;
            break;

        case WRITE_DATA_PARITY_ERROR:
            *ctlParityConfigReg3 |= DDRSS_CTL_PCFG3_REGPORT_WRITE_PARITY_PROTECTION_INJECTION_EN;
            break;

        default:
            status = IPFMA_E_PARAM;
            break;
    }

    return status;
}

int32_t IpFma_Ddr17_DisableCtlParityProtectionInjection(uint32_t baseAddress, IpFma_Ddr17CtlParityErrorSource source)
{
    IpFma_Status status = IPFMA_OK;

    uint32_t* ctlParityConfigReg2 = (uint32_t*) (baseAddress + DDRSS_CTL_PCFG1_REG_CTL_443_OFFSET);
    uint32_t* ctlParityConfigReg3 = (uint32_t*) (baseAddress + DDRSS_CTL_PCFG3_REG_CTL_444_OFFSET);

    switch (source)
    {
        case ADDRESS_PARITY_ERROR:
            *ctlParityConfigReg2 &= ~(DDRSS_CTL_PCFG2_REGPORT_ADDR_PARITY_PROTECTION_INJECTION_EN);
            break;

        case WRITE_DATA_MASK_PARITY_ERROR:
            *ctlParityConfigReg2 &= ~(DDRSS_CTL_PCFG2_REGPORT_WRITEMASK_PARITY_PROTECTION_INJECTION_EN);
            break;

        case PARAM_REGISTER_PARITY_ERROR:
            *ctlParityConfigReg3 &= ~(DDRSS_CTL_PCFG3_PARAMREG_PARITY_PROTECTION_INJECTION_EN);
            break;

        case READ_DATA_PARITY_ERROR:
            *ctlParityConfigReg3 &= ~(DDRSS_CTL_PCFG3_REGPORT_READ_PARITY_PROTECTION_INJECTION_EN);
            break;

        case WRITE_DATA_PARITY_ERROR:
            *ctlParityConfigReg3 &= ~(DDRSS_CTL_PCFG3_REGPORT_WRITE_PARITY_PROTECTION_INJECTION_EN);
            break;

        default:
            status = IPFMA_E_PARAM;
            break;
    }

    return status;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/** @} */