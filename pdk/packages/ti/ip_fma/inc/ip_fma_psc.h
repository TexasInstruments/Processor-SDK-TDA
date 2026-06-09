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
 *  \defgroup IP_FMA_PSC The Power and Sleep Controller
 *            static Register Check Wrappers
 */

/**
 *  \ingroup  IP_FMA_PSC
 *  \defgroup IP_FMA_PSC_INTERFACE PSC Register Check Interface
 *
 *  @{
 */

/**
 *  \file     ip_fma_psc.h
 *
 *  \brief    PSC-specific wrappers over IP_FMA Register Check
 *            for readback of static configuration registers.
 *
 */

#ifndef IP_FMA_PSC_H_
#define IP_FMA_PSC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ip_fma_common.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Number of power domains in the J7AEP and J7AHP MAIN Domain */
#define PSC_PD_NUM            (43U)

/** \brief Number of modules in the J7AEP and J7AHP MAIN Domain */
#define PSC_MD_NUM            (128U)

/** \brief Number of groups in Power Domain Transition Command (PD 0-31, 32-63) */
#define PSC_PTCMD_GR_NUM      (2U)

/** \brief Number of MCU power domains in the WKUP Domain */
#define PSC_MCU_PD_NUM         (2U)

/** \brief Number of MCU modules in the WKUP Domain */
#define PSC_MCU_MD_NUM         (22U)

#if defined(SOC_J784S4)
/** \brief Number of power domains in the Bolt-on domain */
#define PSC_BOLTON_PD_NUM      (8U)
/** \brief Number of modules in the Bolt-on domain */
#define PSC_BOLTON_MD_NUM      (27U)
#endif


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief PSC (MAIN domain) registers snapshot/expected set for RESET.
 */
typedef struct
{
    uint32_t psc_ptcmd[PSC_PTCMD_GR_NUM]; /* Power Domain Transition Command */
    uint32_t psc_pdctl[PSC_PD_NUM];       /* Power Domain Control */
    uint32_t psc_pdcfg[PSC_PD_NUM];       /* Power Domain Configuration */
    uint32_t psc_mdcfg[PSC_MD_NUM];       /* Module Configuration */
    uint32_t psc_mdctl[PSC_MD_NUM];       /* Module Control */
} IpFma_PscRegs;

/**
 * \brief PSC (MCU domain) registers snapshot/expected set for RESET.
 */
typedef struct
{
    uint32_t psc_mcu_ptcmd;                   /* Power Domain Transition Command */
    uint32_t psc_mcu_pdctl[PSC_MCU_PD_NUM];   /* Power Domain Control */
    uint32_t psc_mcu_pdcfg[PSC_MCU_PD_NUM];   /* Power Domain Configuration */
    uint32_t psc_mcu_mdcfg[PSC_MCU_MD_NUM];   /* Module Configuration */
    uint32_t psc_mcu_mdctl[PSC_MCU_MD_NUM];   /* Module Control */
} IpFma_PscMcuRegs;

#if defined(SOC_J784S4)
/**
 * \brief PSC (Bolt-on domain) registers snapshot/expected set for RESET.
 */
typedef struct
{
    uint32_t psc_bolton_ptcmd;
    uint32_t psc_bolton_pdctl[PSC_BOLTON_PD_NUM];
    uint32_t psc_bolton_pdcfg[PSC_BOLTON_PD_NUM];
    uint32_t psc_bolton_mdcfg[PSC_BOLTON_MD_NUM];
    uint32_t psc_bolton_mdctl[PSC_BOLTON_MD_NUM];
} IpFma_PscBoltonRegs;
#endif

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Read PSC registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Psc_GetRegs(uintptr_t baseAddr, IpFma_PscRegs* out);

/**
 * \brief Compare PSC registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Psc_CompareRegs(const IpFma_PscRegs* expected,
                                    const IpFma_PscRegs* actual);

/**
 * \brief Read MCU domain PSC registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Psc_GetMcuRegs(uintptr_t baseAddr, IpFma_PscMcuRegs* out);

/**
 * \brief Compare MCU domain PSC registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Psc_CompareMcuRegs(const IpFma_PscMcuRegs* expected,
                                        const IpFma_PscMcuRegs* actual);

#if defined(SOC_J784S4)
/**
 * \brief Read Bolt-on domain PSC registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Psc_GetBoltonRegs(uintptr_t baseAddr, IpFma_PscBoltonRegs* out);

/**
 * \brief Compare Bolt-on domain PSC registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Psc_CompareBoltonRegs(const IpFma_PscBoltonRegs* expected,
                                          const IpFma_PscBoltonRegs* actual);
#endif
/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_PSC_H_ */

/** @} */
