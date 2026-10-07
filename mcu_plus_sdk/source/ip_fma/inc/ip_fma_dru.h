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
 *  \defgroup IP_FMA_DRU DRU module Register Check Wrappers
 */

/**
 *  \ingroup  IP_FMA_DRU
 *  \defgroup IP_FMA_DRU_INTERFACE DRU module Register Check Interface
 *
 *  @{
 */

/**
 *  \file     ip_fma_dru.h
 *
 *  \brief    DRU-specific wrappers over IP_FMA Register Check.
 *
 */

#ifndef IP_FMA_DRU_H_
#define IP_FMA_DRU_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ip_fma_common.h"
#include <drivers/hw_include/dru/v2/csl_dru.h>
#include <drivers/hw_include/dru/v2/cslr_dru.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Number of DRU queues */
#define DRU_NUM_QUEUES      (CSL_DRU_NUM_QUEUE)

/** \brief Number of DRU channels */
#define DRU_NUM_CHANNELS    (CSL_DRU_NUM_CH)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief DRU Queue Registers structure
 */
typedef struct
{
    uint64_t cfg[DRU_NUM_QUEUES];             /* Configuration Register for Queue 0 */
} IpFma_DruQueueRegs;

/**
 *  \brief DRU CHNRT Registers structure
 */
typedef struct
{
    uint64_t chst_sched[DRU_NUM_CHANNELS];   /* Channel Static Scheduler Config Register */
} IpFma_DruChnrtRegs;


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Read DRU queue registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Dru_GetQueueRegs(uintptr_t baseAddr, IpFma_DruQueueRegs* out);

/**
 * \brief Compare DRU queue registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match,
 *         \ref IPFMA_E_MISMATCH on first mismatch,
 *         or another error code.
 */
IpFma_Status IpFma_Dru_CompareQueueRegs(const IpFma_DruQueueRegs* expected,
                                              const IpFma_DruQueueRegs* actual);

/**
 * \brief Read DRU chnrt registers into \c out.
 *
 * \param baseAddr Pointer to the base address of the group of registers.
 * \param out      Output structure to be filled with actual values.
 *
 * \return \ref IPFMA_OK on success, error code otherwise.
 */
IpFma_Status IpFma_Dru_GetChnrtRegs(uintptr_t baseAddr, IpFma_DruChnrtRegs* out);

/**
 * \brief Compare DRU chnrt registers: \c expected vs \c actual (no HW).
 *
 * \param expected  Input structure with expected values.
 * \param actual    Input structure with actual values (from Get).
 *
 * \return \ref IPFMA_OK if all match,
 *         \ref IPFMA_E_MISMATCH on first mismatch,
 *         or another error code.
 */
IpFma_Status IpFma_Dru_CompareChnrtRegs(const IpFma_DruChnrtRegs* expected,
                                              const IpFma_DruChnrtRegs* actual);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IP_FMA_DRU_H_ */

/** @} */
