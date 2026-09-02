/**
 *   Copyright (c) Texas Instruments Incorporated 2018
 *   All rights reserved.
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
 *  \file vhwa_utils.h
 *
 *  \brief File containing utility APIs
 *
 */

#ifndef VHWA_UTILS_H_
#define VHWA_UTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <vhwa_vpac_abstraction_layer.h>
#include <vhwa_dmpac_abstraction_layer.h>
#ifdef CSL_SOC_PATH
#endif

/* UTILS is currently enabling irq in INTD module,
 * so including VPAC CSLR here.
 * TODO: Move interrupt enable in INTD to common place */
#include <soc/vhwa_soc.h>
#if !defined(SOC_AM62A) && !defined(SOC_AM62AX)
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/**< SL2 Instance ID for VPAC */
#define VHWA_SL2_INST_VPAC      (1U)
/**< SL2 Instance ID for DMPAC */
#define VHWA_SL2_INST_DMPAC     (2U)

#define VHWA_VPAC0_VISS_LOCK_IDX        (0u)
#define VHWA_VPAC0_LDC_LOCK_IDX         (1u)
#define VHWA_VPAC0_MSC0_LOCK_IDX        (2u)
#define VHWA_VPAC0_MSC1_LOCK_IDX        (3u)
#define VHWA_VPAC0_NF_LOCK_IDX          (4u)

#define VHWA_VPAC1_VISS_LOCK_IDX        (5u)
#define VHWA_VPAC1_LDC_LOCK_IDX         (6u)
#define VHWA_VPAC1_MSC0_LOCK_IDX        (7u)
#define VHWA_VPAC1_MSC1_LOCK_IDX        (8u)
#define VHWA_VPAC1_NF_LOCK_IDX          (9u)


#define VHWA_DMPAC_DOF_LOCK_IDX         (10u)
#define VHWA_DMPAC_SDE_LOCK_IDX         (11u)

#define VHWA_MAX_LOCK                   (12u)

/**
 * @brief Flags to enable or configure the safety mechanism.
 *
 * The safety mechanism can operate in the following modes:
 * - One-shot mode: The mechanism is triggered once and then disabled until re-enabled.
 * - Continuous mode: The mechanism remains active and continuously monitors or enforces safety.
 * - Disabled mode: The safety mechanism is turned off and does not perform any checks or actions.
 *
 * @param enableReconfigReinitReg
 *        If set, triggers re-initialization and re-configuration of the registers.
 *
 * @param enableStatusRegValidate
 *        If set, enables validation of the status register to ensure system integrity.
 *
 * @param enableConfigRegValidate
 *        If set, enables validation of the configuration register to detect misconfiguration.
 */
/* ========================================================================== */
/*                           Safety Mechanism Modes                           */
/* ========================================================================== */
/**
 *  \brief Safety mechanism mode: Disabled
 */
#define VHWA_SAFETY_MODE_DISABLED    (0U)

/**
 *  \brief Safety mechanism mode: One-shot enable
 */
#define VHWA_SAFETY_MODE_ONESHOT     (1U)

/**
 *  \brief Safety mechanism mode: Continuous enable
 */
#define VHWA_SAFETY_MODE_CONTINUOUS  (2U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

uint32_t Vhwa_calcSl2Pitch(uint32_t imgPitch);

uint32_t Vhwa_ceil(uint32_t num, uint32_t den);

uint64_t Vhwa_allocateSl2(uint32_t allocSize, uint32_t sl2Inst);

/**
 * \brief Allocate SL2 memory with specified alignment requirement
 *
 * Allocates SL2 memory from VPAC or DMPAC SL2 pool with guaranteed alignment.
 * The returned address will be aligned to the specified alignment boundary.
 *
 * \param allocSize  [IN] Size to allocate in bytes
 * \param sl2Inst    [IN] SL2 instance (VHWA_SL2_INST_VPAC or VHWA_SL2_INST_DMPAC)
 * \param alignment  [IN] Alignment requirement in bytes (MUST be power of 2)
 *                        Valid values: 16, 64, 256, 4096, 16384 (0x4000), etc.
 *                        Common use: 0x4000u (16KB) for firewall-protected regions
 *
 * \return Aligned SL2 address on success, 0 on failure
 *
 * \note
 *  - alignment MUST be a power of 2 (1, 2, 4, 8, 16, ..., 16384, ...)
 *  - Non-power-of-2 alignment values will cause allocation to fail
 *  - Function internally rounds up the allocation to account for alignment overhead
 *  - Use enable16KBAlignment flag in SL2 allocation parameters to request 16KB alignment
 *
 * \warning Using non-power-of-2 alignment will return NULL (allocation failure)
 */
uint64_t Vhwa_allocateSl2Aligned(uint32_t allocSize, uint32_t sl2Inst, uint32_t alignment);

void Vhwa_FreeSl2(uint32_t sl2Inst);

void Vhwa_enableHtsIntr(CSL_vpac_intd_cfgRegs *intdRegs, uint32_t irqNum,
    uint32_t htsPipelineNum);
void Vhwa_disableHtsIntr(CSL_vpac_intd_cfgRegs *intdRegs, uint32_t irqNum,
    uint32_t htsPipelineNum);
void Vhwa_clearHtsIntr(CSL_vpac_intd_cfgRegs *intdRegs, uint32_t irqNum,
    uint32_t htsPipelineNum);
void vhwa_GetErrIntrStat(volatile const uint32_t *intdErrReg,volatile uint32_t *intdClrReg,
    uint32_t *eeStat, uint32_t errEventsMask);
void vhwa_GetWdtimerErrIntrStat(const volatile uint32_t *intdErrReg,volatile uint32_t *intdClrReg,
    uint32_t *wdTimer_errStat, uint32_t wdTimer_bit);

#if !defined(SOC_AM62A)
void Vhwa_enableDmpacHtsIntr(CSL_dmpac_intd_cfgRegs *intdRegs, uint32_t irqNum,
    uint32_t htsPipelineNum);
void Vhwa_disableDmpacHtsIntr(CSL_dmpac_intd_cfgRegs *intdRegs, uint32_t irqNum,
    uint32_t htsPipelineNum);
void Vhwa_clearDmpacHtsIntr(CSL_dmpac_intd_cfgRegs *intdRegs, uint32_t irqNum,
    uint32_t htsPipelineNum);
#endif

int32_t Vhwa_commonCreateHwaLock(uint32_t idx);
void Vhwa_commonDeleteHwaLock(uint32_t idx);
int32_t Vhwa_commonHwaLockAquire(uint32_t idx, uint32_t timeout);
void Vhwa_commonHwaLockRelease(uint32_t idx);

#if !defined(SOC_AM62A) /* LBIST not applicable for am62a */
int32_t Vhwa_m2mDofReInit(void);
int32_t Vhwa_m2mSdeReInit(void);
int32_t Vhwa_m2mVissReInit(void);
int32_t Vhwa_m2mLdcReInit(void);
int32_t Vhwa_m2mMscReInit(void);
int32_t Vhwa_m2mNfReInit(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* VHWA_UTILS_H_ */