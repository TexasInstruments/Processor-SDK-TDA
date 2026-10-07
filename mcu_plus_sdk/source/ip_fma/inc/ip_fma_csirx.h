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
 *  \defgroup IP_FMA_CSIRX CSIRX module Register Check Wrappers
 */

/**
 *  \ingroup  IP_FMA_CSIRX
 *  \defgroup IP_FMA_CSIRX_INTERFACE CSIRX module Register Check Interface
 *
 *  @{
 */

/**
 *  \file     ip_fma_csirx.h
 *
 *  \brief    CSIRX-specific wrappers over IP_FMA Register Check.
 *
 */

#ifndef IP_FMA_CSIRX_H_
#define IP_FMA_CSIRX_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ip_fma_common.h"

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Shim group register snapshot/expected set for CSIRX.
 */
typedef struct
{
    uint32_t csirx_id;
    uint32_t vp0;           /* Video Port 0 config                  */
    uint32_t vp1;           /* Video Port 1 config                  */
    uint32_t cntl;          /* Control register for csi rx wrapper  */
    uint32_t dmacntx[32];   /* DMA Channel Context                  */
    uint32_t psi_cfg0[32];  /* PSI configuration register0          */
    uint32_t psi_cfg1[32];  /* PSI configuration register1          */
} IpFma_CsirxShimRegs;

/**
 * \brief D-PHY register snapshot/expected set for CSIRX.
 */
typedef struct
{
    uint32_t pcs_tx_dig_tbit0;
    uint32_t pcs_tx_dig_tbit2;
    uint32_t pcs_tx_dig_tbit3;
    uint32_t cmn0_cmn_dig_tbit2;
    uint32_t iso_phy_iso_cl_cntrl_l;
    uint32_t iso_phy_iso_dl_ctrl_l0;
    uint32_t iso_phy_iso_dl_ctrl_l1;
    uint32_t iso_ldd_phy_iso_dl_ctrl_l2;
    uint32_t iso_ldd_phy_iso_dl_ctrl_l3;
    uint32_t cmn0_cmn_dig_tbit56;
    uint32_t cmn0_cmn_dig_tbit35;
    uint32_t cmn0_cmn_dig_tbit36;
} IpFma_CsirxDphyRegs;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Read CSIRX D-PHY registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Csirx_GetDphyRegs(uintptr_t baseAddr, IpFma_CsirxDphyRegs* out);

/**
 * \brief Compare CSIRX D-PHY registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Csirx_CompareDphyRegs(const IpFma_CsirxDphyRegs* expected,
                                         const IpFma_CsirxDphyRegs* actual);

/**
 * \brief Read CSIRX SHIM registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Csirx_GetShimRegs(uintptr_t baseAddr, IpFma_CsirxShimRegs* out);

/**
 * \brief Compare CSIRX SHIM registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Csirx_CompareShimRegs(const IpFma_CsirxShimRegs* expected,
                                         const IpFma_CsirxShimRegs* actual);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_CSIRX_H_ */

/** @} */
