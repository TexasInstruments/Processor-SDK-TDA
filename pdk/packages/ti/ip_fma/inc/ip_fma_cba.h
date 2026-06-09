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
 *  \defgroup IP_FMA_CBA_H Register Readback of CBA
 */

/**
 *  \ingroup  IP_FMA_CBA_H
 *  \defgroup IP_FMA_CBA_H CBA Register Check Interface
 *
 *  @{
 */

/**
 *  \file     ip_fma_cba.h
 *
 *  \brief    CBA specific wrappers over IP_FMA Common Register Check
 *            for readback of selected CBA static configuration registers.
 *
 */

#ifndef IP_FMA_CBA_H_
#define IP_FMA_CBA_H_

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
 * \brief Structure representing a single configurable region for the CBA
 *        Firewall register set.
 */
typedef struct
{
    uint32_t control;
    uint32_t permission[3];
    uint32_t start_address_l;
    uint32_t start_address_h;
    uint32_t end_address_l;
    uint32_t end_address_h;
} IpFma_CbaFwRegsRegion;

/**
 * \brief Structure representing the full set of the CBA Firewall regions.
 */
typedef struct
{
    IpFma_CbaFwRegsRegion region[4];
} IpFma_CbaFwRegs;

/**
 * \brief Structure representing a single configurable channel for the
 *        CBA Firewall Channel register set.
 */
typedef struct
{
    uint32_t control;
    uint32_t permission[2];
} IpFma_CbaFwchRegsRegionChannel;

/**
 * \brief Sructure representing the full set of CBA Firewall Channel channels.
 */
typedef struct
{
    IpFma_CbaFwchRegsRegionChannel region_0_channel[32];
} IpFma_CbaFwchRegs;

/**
 * \brief Structure representing a single configurable region for the 
 *        CBA ISC register set.
 */
typedef struct
{
    uint32_t control;
    uint32_t start_address_l;
    uint32_t end_address_h;
} IpFma_CbaIscRegsRegion;

/**
 * \brief Structure representing the full set of CBA ISC registers
 *        including all the regions.
 */
typedef struct
{
    IpFma_CbaIscRegsRegion region[4];
    uint32_t region_def_control;
} IpFma_CbaIscRegs;


/**
 * \brief Structure representing the specific QOS/Map CBA static registers.
 */
typedef struct 
{
    uint32_t grp_map1;
    uint32_t grp_map2;
    uint32_t map0;   
} IpFma_CbaMapRegs;

/**
 * \brief Structure representing the specific Error CBA static registers
 */
typedef struct 
{
    uint32_t pid;
    uint32_t destination_id;
    uint32_t eoi;
} IpFma_CbaErrRegs;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Read CBA Firewall registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Cba_GetFwRegs(uintptr_t baseAddr, IpFma_CbaFwRegs* out);

/**
 * \brief Compare CBA Firewall registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Cba_CompareFwRegs(const IpFma_CbaFwRegs* expected,
                                     const IpFma_CbaFwRegs* actual);

/**
 * \brief Read CBA Firewall Channel registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Cba_GetFwchRegs(uintptr_t baseAddr, IpFma_CbaFwchRegs* out);

/**
 * \brief Compare CBA Firewall Channel registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Cba_CompareFwchRegs(const IpFma_CbaFwchRegs* expected,
                                       const IpFma_CbaFwchRegs* actual);

/**
 * \brief Read CBA ISC registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Cba_GetIscRegs(uintptr_t baseAddr, IpFma_CbaIscRegs* out);

/**
 * \brief Compare CBA ISC registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Cba_CompareIscRegs(const IpFma_CbaIscRegs* expected,
                                      const IpFma_CbaIscRegs* actual);

/**
 * \brief Read CBA module QOS/Map registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Cba_GetMapRegs(uintptr_t baseAddr, IpFma_CbaMapRegs* out);

/**
 * \brief Compare CBA module QOS/Map registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Cba_CompareMapRegs(const IpFma_CbaMapRegs* expected,
                                      const IpFma_CbaMapRegs* actual);

/**
 * \brief Read CBA Error registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Cba_GetErrRegs(uintptr_t baseAddr, IpFma_CbaErrRegs* out);

/**
 * \brief Compare CBA Error registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match.
 *         \ref IPFMA_E_MISMATCH on first mismatch
 *              or another error code.
 */
IpFma_Status IpFma_Cba_CompareErrRegs(const IpFma_CbaErrRegs* expected,
                                      const IpFma_CbaErrRegs* actual);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_CBA_H_ */

/** @} */