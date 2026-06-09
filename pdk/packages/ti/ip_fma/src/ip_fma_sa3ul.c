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
 *  \file     ip_fma_sa3ul.c
 *
 *  \brief    This file contains SA3UL library functions implementations
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <cslr.h>

#include <ti/csl/cslr_cp_ace.h>

#include <ip_fma_common.h>
#include <ip_fma_sa3ul.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None*/

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* STATIC REGISTERS */
/** \brief Array containing the offsets of the MMRA region(PKA and TRNG) static configuration registers in . */
static const uint32_t gIpFmaSa3ul_mmraRegOffsets[] =
{
    CSL_CP_ACE_UPDATES_TRNG_INTR_SET,
    CSL_CP_ACE_UPDATES_PKA_INTR_SET,
};

/** \brief Array containing the offsets of the EIP_76d region(TRNG) static configuration registers. */
static const uint32_t gIpFmaSa3ul_trngRegOffsets[] =
{
    CSL_CP_ACE_TRNG_CONFIG,
    CSL_CP_ACE_TRNG_ALARMCNT,
    CSL_CP_ACE_TRNG_FRODETUNE,
    CSL_CP_ACE_TRNG_OPTIONS,
    CSL_CP_ACE_TRNG_COUNT
};

/** \brief Array containing the offsets of the EIP_29t2 region(PKA) static configuration registers. */
static const uint32_t gIpFmaSa3ul_pkaRegOffsets[] =
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
static const IpFma_Sa3ulRegData gIpFmaSa3ul_staticRegData[] =
{
    {IP_FMA_SA3UL_MMRA_REGION_BASE_ADDRESS,     gIpFmaSa3ul_mmraRegOffsets, IP_FMA_SA3UL_MMRA_STATIC_REG_COUNT},
    {IP_FMA_SA3UL_EIP_76D_REGION_BASE_ADDRESS,  gIpFmaSa3ul_trngRegOffsets, IP_FMA_SA3UL_EIP76D_STATIC_REG_COUNT},
    {IP_FMA_SA3UL_EIP_29T2_REGION_BASE_ADDRESS, gIpFmaSa3ul_pkaRegOffsets,  IP_FMA_SA3UL_EIP29T2_STATIC_REG_COUNT},
};


/* DYNAMIC REGISTERS */
/** \brief Array containing the offsets of the MMRS region dynamic configuration registers. */
static const uint32_t gIpFmaSa3ul_mmrsDynRegOffsets[] =
{
    CSL_CP_ACE_CTXCACH_CTRL,
    CSL_CP_ACE_CTXCACH_SC_ID
};

/** \brief Array containing the offsets of the MMRA region dynamic configuration registers. */
static const uint32_t gIpFmaSa3ul_mmraDynRegOffsets[] =
{
    CSL_CP_ACE_UPDATES_EXCEPTION_LOGGING_HEADER0,
    CSL_CP_ACE_UPDATES_EXCEPTION_PEND_CLEAR,
    CSL_CP_ACE_UPDATES_TRNG_INTR_CLEAR,
    CSL_CP_ACE_UPDATES_PKA_INTR_CLEAR,
    CSL_CP_ACE_UPDATES_KEK(0),
    CSL_CP_ACE_UPDATES_KEK(1),
    CSL_CP_ACE_UPDATES_KEK(2),
    CSL_CP_ACE_UPDATES_KEK(3),
    CSL_CP_ACE_UPDATES_KEK(4),
    CSL_CP_ACE_UPDATES_KEK(5),
    CSL_CP_ACE_UPDATES_KEK(6),
    CSL_CP_ACE_UPDATES_KEK(7),
    CSL_CP_ACE_UPDATES_KEK_LOCK
};

/** \brief Array containing the offsets of the ECC region dynamic configuration registers. */
static const uint32_t gIpFmaSa3ul_eccDynRegOffsets[] =
{
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(0),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(1),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(2),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(3),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(4),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(5),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(6),
    IP_FMA_SA3UL_ECC_RESERVED_SBUS_(7),
};

/** \brief Array containing the offsets of the EIP_76d region dynamic configuration registers. */
static const uint32_t gIpFmaSa3ul_eip76dDynRegOffsets[] =
{
    CSL_CP_ACE_TRNG_INPUT_0,
    CSL_CP_ACE_TRNG_INPUT_1,
    CSL_CP_ACE_TRNG_INPUT_2,
    CSL_CP_ACE_TRNG_INPUT_3,
    CSL_CP_ACE_TRNG_STATUS,
    CSL_CP_ACE_TRNG_INTACK_SECURE_MODE,
    CSL_CP_ACE_TRNG_INTACK,
    CSL_CP_ACE_TRNG_ALARMCNT,
    CSL_CP_ACE_TRNG_RAW_L,
    CSL_CP_ACE_TRNG_RAW_H,
    CSL_CP_ACE_TRNG_SPB_TESTS,
    CSL_CP_ACE_TRNG_COUNT,
    CSL_CP_ACE_TRNG_PS_AI_0,
    CSL_CP_ACE_TRNG_PS_AI_1,
    CSL_CP_ACE_TRNG_PS_AI_2,
    CSL_CP_ACE_TRNG_PS_AI_3,
    CSL_CP_ACE_TRNG_PS_AI_4,
    CSL_CP_ACE_TRNG_PS_AI_5,
    CSL_CP_ACE_TRNG_PS_AI_6,
    CSL_CP_ACE_TRNG_PS_AI_7,
    CSL_CP_ACE_TRNG_PS_AI_8,
    CSL_CP_ACE_TRNG_PS_AI_9,
    CSL_CP_ACE_TRNG_PS_AI_10,
    CSL_CP_ACE_TRNG_PS_AI_11,
    CSL_CP_ACE_TRNG_TEST,
    CSL_CP_ACE_TRNG_BLOCKCNT,
};

/** \brief Array containing the offsets of the EIP_29t2 region dynamic configuration registers. */
static const uint32_t gIpFmaSa3ul_eip29t2DynRegOffsets[] =
{
    CSL_CP_ACE_PKA_EIP29T2_PKA_APTR,
    CSL_CP_ACE_PKA_EIP29T2_PKA_BPTR,
    CSL_CP_ACE_PKA_EIP29T2_PKA_CPTR,
    CSL_CP_ACE_PKA_EIP29T2_PKA_DPTR,
    CSL_CP_ACE_PKA_EIP29T2_PKA_ALENGTH,
    CSL_CP_ACE_PKA_EIP29T2_PKA_BLENGTH,
    CSL_CP_ACE_PKA_EIP29T2_PKA_SHIFT,
    CSL_CP_ACE_PKA_EIP29T2_PKA_FUNCTION,
    CSL_CP_ACE_PKA_EIP29T2_PKA_COMPARE,
    CSL_CP_ACE_PKA_EIP29T2_PKA_MSW,
    CSL_CP_ACE_PKA_EIP29T2_PKA_DIVMSW,
    CSL_CP_ACE_PKA_EIP29T2_LNME1_STATUS,
    CSL_CP_ACE_PKA_EIP29T2_LNME1_CONTROL,
    CSL_CP_ACE_PKA_EIP29T2_LNME1_NBASE,
    CSL_CP_ACE_PKA_EIP29T2_LNME1_NACC,
    CSL_CP_ACE_PKA_EIP29T2_LNME0_CONTROL,
    CSL_CP_ACE_PKA_EIP29T2_LNME_DATAPATH,
    CSL_CP_ACE_PKA_EIP29T2_LNME_FAST_CTRL,
    CSL_CP_ACE_PKA_EIP29T2_LNME_FAST_STRT,
    CSL_CP_ACE_PKA_EIP29T2_LNME_FAST_MMM,
    CSL_CP_ACE_PKA_EIP29T2_LNME0_NBASE,
    CSL_CP_ACE_PKA_EIP29T2_LNME0_NACC,
    CSL_CP_ACE_PKA_EIP29T2_GF2M_CMD,
    CSL_CP_ACE_PKA_EIP29T2_GF2M_STAT,
    CSL_CP_ACE_PKA_EIP29T2_PKA_CLK_CTRL,
    CSL_CP_ACE_PKA_EIP29T2_PKA_SYSCONFIG,
    CSL_CP_ACE_PKA_EIP29T2_PKA_IRQSTATUS,
};

/** \brief Array containing the base addresses and register offsets for SA3UL dynamic registers. */
static const IpFma_Sa3ulRegData gIpFmaSa3ul_dynamicRegData[] =
{
    {IP_FMA_SA3UL_MMRS_REGION_BASE_ADDRESS,     gIpFmaSa3ul_mmrsDynRegOffsets,    IP_FMA_SA3UL_MMRS_DYNAMIC_REG_COUNT},
    {IP_FMA_SA3UL_MMRA_REGION_BASE_ADDRESS,     gIpFmaSa3ul_mmraDynRegOffsets,    IP_FMA_SA3UL_MMRA_DYNAMIC_REG_COUNT},
    {IP_FMA_SA3UL_ECC_AGGR_REGION_BASE_ADDRESS, gIpFmaSa3ul_eccDynRegOffsets,     IP_FMA_SA3UL_ECC_DYNAMIC_REG_COUNT},
    {IP_FMA_SA3UL_EIP_76D_REGION_BASE_ADDRESS,  gIpFmaSa3ul_eip76dDynRegOffsets,  IP_FMA_SA3UL_EIP_76D_DYNAMIC_REG_COUNT},
    {IP_FMA_SA3UL_EIP_29T2_REGION_BASE_ADDRESS, gIpFmaSa3ul_eip29t2DynRegOffsets, IP_FMA_SA3UL_EIP_29T2_DYNAMIC_REG_COUNT},
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

IpFma_Status IpFma_Sa3ul_GetStaticRegsCfg(uintptr_t *regCfg)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t mmrRegionsCount = (sizeof(gIpFmaSa3ul_staticRegData) / sizeof(IpFma_Sa3ulRegData));

    status = IpFma_Sa3ul_GetRegsCfg(regCfg,
                                    IP_FMA_SA3UL_STATIC_REGDUMP_BUFFER_SIZE,
                                    gIpFmaSa3ul_staticRegData,
                                    mmrRegionsCount);

    return (status);
}

IpFma_Status IpFma_Sa3ul_GetDynamicRegsCfg(uintptr_t *regCfg)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t mmrRegionsCount = (sizeof(gIpFmaSa3ul_dynamicRegData) / sizeof(IpFma_Sa3ulRegData));

    status = IpFma_Sa3ul_GetRegsCfg(regCfg,
                                    IP_FMA_SA3UL_DYNAMIC_REGDUMP_BUFFER_SIZE,
                                    gIpFmaSa3ul_dynamicRegData,
                                    mmrRegionsCount);

    return (status);
}

IpFma_Status IpFma_Sa3ul_VerifyStaticRegsCfg(const uintptr_t *goldenRefRegCfg)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t mmrRegionsCount = (sizeof(gIpFmaSa3ul_staticRegData) / sizeof(IpFma_Sa3ulRegData));
    uintptr_t actualRegCfg[IP_FMA_SA3UL_STATIC_REGDUMP_BUFFER_SIZE];

    /* Read SA3UL register data from memory. */
    status = IpFma_Sa3ul_GetRegsCfg(actualRegCfg,
                                    IP_FMA_SA3UL_STATIC_REGDUMP_BUFFER_SIZE,
                                    gIpFmaSa3ul_staticRegData,
                                    mmrRegionsCount);

    status = IpFma_Sa3ul_VerifyRegsCfg(goldenRefRegCfg,
                                       actualRegCfg,
                                       IP_FMA_SA3UL_STATIC_REGDUMP_BUFFER_SIZE);

    return (status);
}

IpFma_Status IpFma_Sa3ul_VerifyDynamicRegsCfg(const uintptr_t *goldenRefRegCfg)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t mmrRegionsCount = (sizeof(gIpFmaSa3ul_dynamicRegData) / sizeof(IpFma_Sa3ulRegData));
    uintptr_t actualRegCfg[IP_FMA_SA3UL_DYNAMIC_REGDUMP_BUFFER_SIZE];

    /* Read SA3UL register data from memory. */
    status = IpFma_Sa3ul_GetRegsCfg(actualRegCfg,
                                    IP_FMA_SA3UL_DYNAMIC_REGDUMP_BUFFER_SIZE,
                                    gIpFmaSa3ul_dynamicRegData,
                                    mmrRegionsCount);

    status = IpFma_Sa3ul_VerifyRegsCfg(goldenRefRegCfg,
                                       actualRegCfg,
                                       IP_FMA_SA3UL_DYNAMIC_REGDUMP_BUFFER_SIZE);

    return (status);
}

IpFma_Status IpFma_Sa3ul_GetRegsCfg(uintptr_t* regCfg,
                                    const uint32_t bufferSize,
                                    const IpFma_Sa3ulRegData* regData,
                                    const uint32_t mmrRegionsCount)
{
    IpFma_Status status = IPFMA_OK;

    /* Check if regCfg is NULL */
    if (NULL == regCfg)
    {
        status = IPFMA_E_PARAM;
    }

    /* Check if the regCfg buffer is to small */
    if(IPFMA_OK == status)
    {
        uint32_t totalNumberOfRegs = 0U;
        for (uint32_t mmrRegion = 0U; mmrRegion < mmrRegionsCount; mmrRegion++)
        {
            totalNumberOfRegs += regData[mmrRegion].regNum;
        }

        /* Check for the buffer size */
        if (totalNumberOfRegs > bufferSize)
        {
            status = IPFMA_E_PARAM;
        }
    }

    if(IPFMA_OK == status)
    {
        uint32_t outRegIndex = 0;
        for (uint32_t mmrRegion = 0U; mmrRegion < mmrRegionsCount; mmrRegion++)
        {
            for (uint32_t regIndex = 0U; regIndex < regData[mmrRegion].regNum; regIndex++)
		    {
                uint32_t regAddress = regData[mmrRegion].baseAddr + \
                                      regData[mmrRegion].regOffsetArr[regIndex];

			    regCfg[outRegIndex] = (uintptr_t)CSL_REG32_RD(regAddress);

                outRegIndex++;
            }
        }
    }

    return (status);
}

IpFma_Status IpFma_Sa3ul_VerifyRegsCfg(const uintptr_t* goldenRefRegCfg,
                                       const uintptr_t* actualRegCfg,
                                       const uint32_t registerCount)
{
    IpFma_Status status = IPFMA_OK;

    /* Check if goldenRefRegCfg is NULL */
    if (NULL == goldenRefRegCfg)
    {
        status = IPFMA_E_PARAM;
    }
    /* Check if actualRegCfg is NULL */
    if (NULL == actualRegCfg)
    {
        status = IPFMA_E_PARAM;
    }

    if (IPFMA_OK == status)
    {
        for (uint32_t regIndex = 0U; regIndex < registerCount; regIndex++)
        {
            if ((uint32_t)(goldenRefRegCfg[regIndex] ^ actualRegCfg[regIndex]))
            {
                status = IPFMA_E_MISMATCH;
            }
        }
    }

    return (status);
}
