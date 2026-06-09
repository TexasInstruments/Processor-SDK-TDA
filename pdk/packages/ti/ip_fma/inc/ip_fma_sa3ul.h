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
 *
 */

/**
 *  \defgroup IP_FMA_SA3UL Register Check Wrappers
 */

 /**
 *  \ingroup  IP_FMA_SA3UL
 *  \defgroup IP_FMA_SA3UL_INTERFACE SA3UL Register Check Interface.
 *
 *  @{
 *
 *    SA3UL library APIs are integrated into the safety application to verify
 *    the SA3UL register configurations and validate the current configurations against
 *    golden reference.
 *
 */

/**
 *  \file     ip_fma_sa3ul.h
 *
 *  \brief    This file contains SA3UL library interfaces and related data structures.
 *
 */

/** @} */

#ifndef IP_FMA_SA3UL_H_
#define IP_FMA_SA3UL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief SA3UL MMR regions offsets */
#define IP_FMA_SA3UL_MMRA_REGION_OFFSET                 (0x0000001000U)
#define IP_FMA_SA3UL_EIP_76D_REGION_OFFSET              (0x0000010000U)
#define IP_FMA_SA3UL_EIP_29T2_REGION_OFFSET             (0x0000020000U)

/** \brief SA3UL MMR regions base addresses. */
#define IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS            (0x0040900000U)
#define IP_FMA_SA3UL_ECC_AGGR_REGION_BASE_ADDRESS       (0x004070C000U)

#define IP_FMA_SA3UL_MMRS_REGION_BASE_ADDRESS           (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS)
#define IP_FMA_SA3UL_MMRA_REGION_BASE_ADDRESS           (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS + \
                                                         IP_FMA_SA3UL_MMRA_REGION_OFFSET)
#define IP_FMA_SA3UL_EIP_76D_REGION_BASE_ADDRESS        (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS + \
                                                         IP_FMA_SA3UL_EIP_76D_REGION_OFFSET)
#define IP_FMA_SA3UL_EIP_29T2_REGION_BASE_ADDRESS       (IP_FMA_SA3UL_MMR_REGION_BASE_ADDRESS + \
                                                         IP_FMA_SA3UL_EIP_29T2_REGION_OFFSET)

/* STATIC REGISTERS */
/** \brief Number of static(PKA and TRNG) registers in MMRA region. */
#define IP_FMA_SA3UL_MMRA_STATIC_REG_COUNT              (2U)
/** \brief Number of static registers in TRNG register group(EIP_76D region). */
#define IP_FMA_SA3UL_EIP76D_STATIC_REG_COUNT            (5U)
/** \brief Number of static registers in PKA register group(EIP_29T2 region). */
#define IP_FMA_SA3UL_EIP29T2_STATIC_REG_COUNT           (7U)

/** \brief Total static register dump size for SA3UL. */
#define IP_FMA_SA3UL_STATIC_REGDUMP_BUFFER_SIZE         (IP_FMA_SA3UL_MMRA_STATIC_REG_COUNT + \
                                                         IP_FMA_SA3UL_EIP29T2_STATIC_REG_COUNT + \
                                                         IP_FMA_SA3UL_EIP76D_STATIC_REG_COUNT)


/* DYNAMIC REGISTERS */
/** \brief Number of dynamic registers in MMRS region. */
#define IP_FMA_SA3UL_MMRS_DYNAMIC_REG_COUNT             (2U)

/** \brief Number of dynamic registers in MMRA region. */
#define IP_FMA_SA3UL_MMRA_DYNAMIC_REG_COUNT             (13U)

/** \brief Number of dynamic registers in ECC region. */
#define IP_FMA_SA3UL_ECC_DYNAMIC_REG_COUNT              (8U)

/** \brief Number of dynamic registers in EIP_76d region. */
#define IP_FMA_SA3UL_EIP_76D_DYNAMIC_REG_COUNT          (26U)

/** \brief Number of dynamic registers in EIP_29t2 region. */
#define IP_FMA_SA3UL_EIP_29T2_DYNAMIC_REG_COUNT         (27U)

/** \brief Total register dump size for SA3UL dynamic registers. */
#define IP_FMA_SA3UL_DYNAMIC_REGDUMP_BUFFER_SIZE        (IP_FMA_SA3UL_MMRS_DYNAMIC_REG_COUNT + \
                                                         IP_FMA_SA3UL_MMRA_DYNAMIC_REG_COUNT + \
                                                         IP_FMA_SA3UL_ECC_DYNAMIC_REG_COUNT + \
                                                         IP_FMA_SA3UL_EIP_76D_DYNAMIC_REG_COUNT + \
                                                         IP_FMA_SA3UL_EIP_29T2_DYNAMIC_REG_COUNT)


/** \brief ECC register offsets aren’t predefined, we define them below. */
#define IP_FMA_SA3UL_ECC_RESERVED_SBUS_(n)              (0x10U + ((n) * 0x4U))

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief   Structure to hold the base address and the offsets of registers
 */
typedef struct
{
    /* Base address of register region */
    const uint32_t baseAddr;
	/* Pointer to the register offset array */
	const uint32_t* regOffsetArr;
    /* Number of registers in the region */
    const uint32_t regNum;
} IpFma_Sa3ulRegData;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief      API to get register configuration for SA3UL module.
 *              Reads the SA3UL register configuration from memory and stores in \c regCfg.
 *
 *  \param      regCfg            [OUT]     Pointer to store the SA3UL register configuration
 *  \param      bufferSize        [IN]      Size of buffer to store register data
 *  \param      regData           [IN]      Holds the base address and the offsets of registers
 *  \param      mmrRegionsCount   [IN]      Number of mmr regions
 *
 *  \return     IPFMA_OK: Hardware registers match the golden configuration.
 *              IPFMA_E_PARAM: Internal error during register reading - invalid params.
 */
IpFma_Status IpFma_Sa3ul_GetRegsCfg(uintptr_t* regCfg,
                                    const uint32_t bufferSize,
                                    const IpFma_Sa3ulRegData* regData,
                                    const uint32_t mmrRegionsCount);

/**
 *  \brief      API to verify register configuration for SA3UL module.
 *              Compares actual \c actualRegCfg SA3UL register configuration
 *              with the expected register configuration in \c goldenRefRegCfg.
 *
 *  \param      goldenRefRegCfg   [IN]      Pointer to SA3UL golden/expected register configuration
 *  \param      actualRegCfg      [IN]      Pointer to SA3UL current register configuration
 *  \param      registerCount     [IN]      Number of sa3ul registers.
 *
 *  \return     IPFMA_OK: Hardware registers match the golden configuration.
 *              IPFMA_E_MISMATCH: One or more register bits have changed state.
 *              IPFMA_E_PARAM: Internal error during register reading - invalid params.
 */
IpFma_Status IpFma_Sa3ul_VerifyRegsCfg(const uintptr_t* goldenRefRegCfg,
                                       const uintptr_t* actualRegCfg,
                                       const uint32_t bufferSize);

/**
 *  \brief      API to get static registers configuration for SA3UL module.
 *              This is a wrapper function for \ref IpFma_Sa3ul_GetRegsCfg.
 *              Reads the SA3UL static register configuration from memory and stores in \c regCfg.
 *
 *  \param      regCfg            [OUT]         Pointer to store the SA3UL register configuration.
 *
 *  \return     IPFMA_OK: Hardware registers match the golden configuration.
 *              IPFMA_E_PARAM: Internal error during register reading - invalid params.
 */
IpFma_Status IpFma_Sa3ul_GetStaticRegsCfg(uintptr_t* regCfg);

/**
 *  \brief      API to get dynamic registers configuration for SA3UL module.
 *              This is a wrapper function for \ref IpFma_Sa3ul_GetRegsCfg.
 *              Reads the SA3UL dynamic register configuration from memory and stores in \c regCfg.
 *
 *  \param      regCfg            [OUT]         Pointer to store the SA3UL register configuration.
 *
 *  \return     IPFMA_OK: Hardware registers match the golden configuration.
 *              IPFMA_E_PARAM: Internal error during register reading - invalid params.
 */
IpFma_Status IpFma_Sa3ul_GetDynamicRegsCfg(uintptr_t* regCfg);

/**
 *  \brief      API to verify static registers configuration for SA3UL module.
 *              This is a wrapper function for \ref IpFma_Sa3ul_VerifyRegsCfg.
 *              Reads the SA3UL static register configuration from memory and
 *              compares with the received/expected register configuration in \c goldenRefRegCfg.
 *
 *  \param      goldenRefRegCfg   [IN]          Pointer to SA3UL golden/expected register configuration.
 *
 *  \return     IPFMA_OK: Hardware registers match the golden configuration.
 *              IPFMA_E_MISMATCH: One or more register bits have changed state.
 *              IPFMA_E_PARAM: Internal error during register reading - invalid params.
 */
IpFma_Status IpFma_Sa3ul_VerifyStaticRegsCfg(const uintptr_t* goldenRefRegCfg);

/**
 *  \brief      API to verify dynamic registers configuration for SA3UL module.
 *              This is a wrapper function for \ref IpFma_Sa3ul_VerifyRegsCfg.
 *              Reads the SA3UL register configuration from memory and
 *              compares with the received/expected register configuration in \c goldenRefRegCfg.
 *
 *  \param      goldenRefRegCfg   [IN]          Pointer to SA3UL golden/expected register configuration.
 *
 *  \return     IPFMA_OK: Hardware registers match the golden configuration.
 *              IPFMA_E_MISMATCH: One or more register bits have changed state.
 *              IPFMA_E_PARAM: Internal error during register reading - invalid params.
 */
IpFma_Status IpFma_Sa3ul_VerifyDynamicRegsCfg(const uintptr_t* goldenRefRegCfg);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_SA3UL_H_ */

/** @} */