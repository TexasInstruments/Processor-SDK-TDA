/*
 *  Copyright (C) 2025 Texas Instruments Incorporated
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
 * \file MpuP_armv7r.h
 *
 * \brief File containing Memory Protection Unit (MPU) API for the ARMv7-R arch
 */

/**
 * Include Guard
 */
#ifndef MPUP_ARMV7R_H
#define MPUP_ARMV7R_H

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/hw_include/tistdtypes.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Get the number of unified MPU regions supported
 *  This function is used to get the number of unified MPU regions supported.
 *
 *  \return The number of unified MPU regions supported
 */
uint32_t MpuP_GetNumRegions( void );

/**
 *  \brief Verify an MPU region that is previously configured
 *  This function is used to verify the previous configure an MPU region API.
 *
 *  The value arguments provided to this function are directly verfied against
 *  the corresponding R5 system register. As a result, the values must be
 *  constructed per the expected register format.
 *
 *  See the "MPU memory region programming registers" section of the ARM Cortex
 *  R5 TRM for more information.
 *
 *  \param regionNum        [IN]    Region number
 *  \param baseAddrRegVal   [IN]    Value written to the MPU Region Base
 *                                  Address Register
 *  \param sizeRegVal       [IN]    Value written to the MPU Size and Enable
 *                                  Register
 *  \param accessCtrlRegVal [IN]    Value written to the MPU Region Access
 *                                  Control Register
 *
 */
int32_t MpuP_VerifyCfgRegion( uint32_t regionNum, uint32_t baseAddrRegVal, uint32_t sizeRegVal, uint32_t accessCtrlRegVal );

/**
 *  \brief Enable/disable an MPU region
 *  This function is used to enable or disable an MPU region.
 *
 *  \param regionNum        [IN]    Region number
 *  \param enable           [IN]    0=The specified MPU region is disabled,
 *                                  otherwise it is enabled
 *
 */
void MpuP_EnableRegion( uint32_t regionNum, uint32_t enable );

/**
 *  \brief Verifies the previously called Enable/disable an MPU region API
 *  This function is used to verify enable or disable an MPU region.
 *
 *  \param regionNum        [IN]    Region number
 *  \param enable   [IN]    0=The specified MPU region is disabled, otherwise
 *                          it is enabled
 *
 *  \return CSL_PASS for success otherwise CSL_EFAIL
 */
int32_t MpuP_VerifyEnableRegion( uint32_t regionNum, uint32_t enable );

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Structure Declarations                   */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* MPUP_ARMV7R_H */