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
 *  \ingroup  IP_FMA_COMMON
 *  \defgroup IP_FMA_COMMON_IMPLEMENTATION IP FMA Register Check Implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_common.c
 *
 *  \brief    Generic register readback implementation (read-only).
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/hw_include/cslr.h>
#include "ip_fma_common.h"

/* ========================================================================== */
/*                       Static Function Declarations                         */
/* ========================================================================== */

/**
 * \brief Read a single register using base address and access width.
 *
 * \param baseAddr  Base address of IP.
 * \param offset    Offset from base.
 * \param width     Access width in BYTES (1,2,4,8).
 * \param out       Output value (zero-extended to uint64_t).
 *
 * \return \ref IPFMA_OK on success,
 *         \ref IPFMA_E_PARAM on invalid args or unsupported width,
 *         \ref IPFMA_E_IO for HW access issues (reserved for future use).
 */
static inline IpFma_Status IpFma_ReadOne(uintptr_t baseAddr,
                                         uint32_t offset,
                                         IpFma_RegWidth width,
                                         uint64_t* out);

/* ========================================================================== */
/*                          Function Definitions                               */
/* ========================================================================== */

IpFma_Status IpFma_GetRegsValues(uintptr_t baseAddr,
                                 IpFma_RegDesc* regs,
                                 uint32_t numRegs)
{
    IpFma_Status status = IPFMA_E_PARAM;
    uint32_t i;

    if (((NULL_PTR == regs)) || ((baseAddr == (uintptr_t)0U)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        for (i = 0U; i < numRegs; ++i)
        {
            uint64_t value = 0U;

            status = IpFma_ReadOne(baseAddr,
                                      regs[i].offset,
                                      regs[i].width,
                                      &value);

            if ((status != IPFMA_OK))
            {
                break;
            }

            regs[i].value = value;
        }
    }

    return status;
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline IpFma_Status IpFma_ReadOne(uintptr_t baseAddr,
                                         uint32_t offset,
                                         IpFma_RegWidth width,
                                         uint64_t* out)
{
    IpFma_Status status = IPFMA_E_PARAM;

    if (((NULL_PTR == out)) || ((baseAddr == (uintptr_t)0U)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        uintptr_t addr = (baseAddr + (uintptr_t)offset);
        switch (width)
        {
            case IPFMA_WIDTH_8:
            {
                volatile uint8_t* p = (volatile uint8_t*)addr;
                *out = (uint64_t)CSL_REG8_RD(p);
                status = IPFMA_OK;
                break;
            }
            case IPFMA_WIDTH_16:
            {
                volatile uint16_t* p = (volatile uint16_t*)addr;
                *out = (uint64_t)CSL_REG16_RD(p);
                status = IPFMA_OK;
                break;
            }
            case IPFMA_WIDTH_32:
            {
                volatile uint32_t* p = (volatile uint32_t*)addr;
                *out = (uint64_t)CSL_REG32_RD(p);
                status = IPFMA_OK;
                break;
            }
            case IPFMA_WIDTH_64:
            {
                volatile uint64_t* p = (volatile uint64_t*)addr;
                *out = (uint64_t)CSL_REG64_RD(p);
                status = IPFMA_OK;
                break;
            }
            default:
            {
                status = IPFMA_E_PARAM;
                break;
            }
        }
    }

    return status;
}

/** @} */
