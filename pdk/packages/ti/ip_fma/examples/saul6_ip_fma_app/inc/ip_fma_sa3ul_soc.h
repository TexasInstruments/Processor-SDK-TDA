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
 *  \file     ip_fma_sa3ul_soc.h
 *
 *
 *  \brief    This file contains J721S2 and J784S4 specific data structures for SA3UL module
 *
 */

#ifndef IP_FMA_SA3UL_SOC_H_
#define IP_FMA_SA3UL_SOC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ip_fma_sa3ul.h>
#include <ti/csl/cslr_cp_ace.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief SA3UL MMR regions offsets */
#define IP_FMA_SA3UL_MMRA_REGION_OFFSET                (0x0000001000U)
#define IP_FMA_SA3UL_EIP_76D_REGION_OFFSET             (0x0000010000U)
#define IP_FMA_SA3UL_EIP_29T2_REGION_OFFSET            (0x0000020000U)

/** \brief SA3UL MMR regions base addresses. */
#define IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS           (0x0040900000U)
#define IP_FMA_SA3UL_ECC_AGGR_REGION_BASE_ADDRESS      (0x004070C000U)

#define IP_FMA_SA3UL_MMRA_REGION_BASE_ADDRESS          (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS + \
                                                        IP_FMA_SA3UL_MMRA_REGION_OFFSET)
#define IP_FMA_SA3UL_EIP_76D_REGION_BASE_ADDRESS       (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS + \
                                                        IP_FMA_SA3UL_EIP_76D_REGION_OFFSET)
#define IP_FMA_SA3UL_EIP_29T2_REGION_BASE_ADDRESS      (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS + \
                                                        IP_FMA_SA3UL_EIP_29T2_REGION_OFFSET)


/** \brief Number of static(PKA and TRNG) registers in MMRA region. */
#define IP_FMA_SA3UL_MMRA_STATIC_REG_COUNT             (2U)
/** \brief Number of static registers in TRNG register group(EIP_76D region). */
#define IP_FMA_SA3UL_EIP76D_STATIC_REG_COUNT           (5U)
/** \brief Number of static registers in PKA register group(EIP_29T2 region). */
#define IP_FMA_SA3UL_EIP29T2_STATIC_REG_COUNT          (7U)

/** \brief Total static register dump size for SA3UL. */
#define IP_FMA_SA3UL_REGDUMP_BUFFER_SIZE               (IP_FMA_SA3UL_MMRA_STATIC_REG_COUNT + \
                                                        IP_FMA_SA3UL_EIP29T2_STATIC_REG_COUNT + \
                                                        IP_FMA_SA3UL_EIP76D_STATIC_REG_COUNT)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \brief Array containing the offsets of the PKA and TRNG  static configuration registers in MMRA region. */
static uint32_t gIpFma_sa3ulMmraRegOffsets[] =
{
    CSL_CP_ACE_UPDATES_TRNG_INTR_SET,
    CSL_CP_ACE_UPDATES_PKA_INTR_SET,
};

/** \brief Array containing the offsets of the TRNG(True Random Number Generator)-EIP_76d static configuration registers. */
static uint32_t gIpFma_sa3ulTrngRegOffsets[] =
{
    CSL_CP_ACE_TRNG_CONFIG,
    CSL_CP_ACE_TRNG_ALARMCNT,
    CSL_CP_ACE_TRNG_FRODETUNE,
    CSL_CP_ACE_TRNG_OPTIONS,
    CSL_CP_ACE_TRNG_COUNT
};

/** \brief Array containing the offsets of the PKA(Public Key Accelerator)-EIP29t2 static configuration registers. */
static uint32_t gIpFma_sa3ulPkaRegOffsets[] =
{
    CSL_CP_ACE_PKA_EIP29T2_PKA_SHIFT,
    CSL_CP_ACE_PKA_EIP29T2_PKA_COMPARE,
    CSL_CP_ACE_PKA_EIP29T2_PKA_MSW,
    CSL_CP_ACE_PKA_EIP29T2_PKA_DIVMSW,
    CSL_CP_ACE_PKA_EIP29T2_PKA_OPTIONS,
    CSL_CP_ACE_PKA_EIP29T2_PKA_SYSSTATUS,
    CSL_CP_ACE_PKA_EIP29T2_PKA_IRQENABLE,
};

/** \brief Array containing register data for SA3UL MMR regions. */
static IpFma_Sa3ulRegData gIpFmaSa3ul_regData[] =
{
    {IP_FMA_SA3UL_MMRA_REGION_BASE_ADDRESS,     gIpFma_sa3ulMmraRegOffsets, IP_FMA_SA3UL_MMRA_STATIC_REG_COUNT   },
    {IP_FMA_SA3UL_EIP_76D_REGION_BASE_ADDRESS,  gIpFma_sa3ulTrngRegOffsets, IP_FMA_SA3UL_EIP76D_STATIC_REG_COUNT },
    {IP_FMA_SA3UL_EIP_29T2_REGION_BASE_ADDRESS, gIpFma_sa3ulPkaRegOffsets,  IP_FMA_SA3UL_EIP29T2_STATIC_REG_COUNT},
};

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef IP_FMA_SA3UL_SOC_H_ */
