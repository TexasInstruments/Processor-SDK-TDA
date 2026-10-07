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
 *  \defgroup IP_FMA_COMMON Register Readback (Common)
 */

/**
 *  \ingroup  IP_FMA_COMMON
 *  \defgroup IP_FMA_COMMON_INTERFACE IP FMA Register Check Interface
 *
 *  @{
 */

/**
 *  \file     ip_fma_common.h
 *
 *  \brief    Generic register readback interface.
 *
 */

#ifndef IP_FMA_COMMON
#define IP_FMA_COMMON

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <drivers/hw_include/csl_types.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \brief Register access width in BYTES (1, 2, 4, 8).
 *
 * Values represent byte width, allowing direct use for alignment/stride math.
 */
typedef enum
{
    IPFMA_WIDTH_8  = 1u,  /**< 8-bit access  (1 byte)   */
    IPFMA_WIDTH_16 = 2u,  /**< 16-bit access (2 bytes)  */
    IPFMA_WIDTH_32 = 4u,  /**< 32-bit access (4 bytes)  */
    IPFMA_WIDTH_64 = 8u   /**< 64-bit access (8 bytes)  */
} IpFma_RegWidth;

/**
 * \brief Register descriptor for hardware readback.
 *
 * For \ref IpFma_GetRegsValues : \c value is OUTPUT (actual HW).
 * Values from narrower accesses are zero-extended into 64 bits.
 */
typedef struct
{
    uint32_t           offset;   /**< Offset from base address (in bytes)        */
    uint64_t           value;    /**< Output: actual value read from HW (64-bit) */
    IpFma_RegWidth     width;    /**< Access width (8/16/32/64 bits) in BYTES    */
} IpFma_RegDesc;

/**
 * \brief IpFma return status.
 */
typedef enum
{
    IPFMA_OK = 0,       /**< Success                                 */
    IPFMA_E_PARAM,      /**< Invalid parameter(s)                    */
    IPFMA_E_IO,         /**< Hardware read error / unsupported width */
    IPFMA_E_MISMATCH    /**< Reserved for wrapper comparisons        */
} IpFma_Status;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Read hardware registers and fill \c regs[i].value with actual values.
 *
 * \param baseAddr  Base address of the target IP instance.
 * \param regs      Pointer to array of descriptors.
 * \param numRegs   Number of entries in \c regs.
 *
 * \return \ref IPFMA_OK on success,
 *         \ref IPFMA_E_PARAM on invalid args,
 *         \ref IPFMA_E_IO on read error.
 */
IpFma_Status IpFma_GetRegsValues(uintptr_t baseAddr,
                                 IpFma_RegDesc* regs,
                                 uint32_t numRegs);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_COMMON */

/** @} */
