/*
 *  Copyright (C) 2023-25 Texas Instruments Incorporated
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

#ifndef UTILSP_H
#define UTILSP_H

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 *  \brief Write BTCM Region Register
 *  This function is used to write BTCM region register.
 *
 *  \param value   [IN]    BTCM register value to be written
 *
 *  \return None
 */
void UtilsP_WriteBTCMRegionRegister(uint32_t value);

/**
 *  \brief Write ATCM Region Register
 *  This function is used to write ATCM region register.
 *
 *  \param value   [IN]    ATCM register value to be written
 *
 *  \return None
 */
void UtilsP_WriteATCMRegionRegister(uint32_t value);

/**
 *  \brief Read BTCM Region Register
 *  This function is used to read BTCM region register.
 *
 *
 *  \return BTCM Region Register value
 */
uint32_t UtilsP_ReadBTCMRegionRegister(void);

/**
 *  \brief Read ATCM Region Register
 *  This function is used to read ATCM region register.
 *
 *
 *  \return ATCM Region Register value
 */
uint32_t UtilsP_ReadATCMRegionRegister(void);

/**
 *  \brief Enable/disable hard error cache
 *  This function is used to enable or disable hard error caches.
 *
 *  \param enable   [IN]    0=Hard error caches are disabled, otherwise
 *                          they are enabled
 *
 *  \return None
 */
void UtilsP_CacheEnableHardErrCache( uint32_t enable );

/**
 *  \brief Executes DSB instruction
 */
void UtilsP_Dsb(void);

/* @} */

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
#endif  /* extern "C" */

#endif  /* end of CSL_ARM_R5_H_ definition */