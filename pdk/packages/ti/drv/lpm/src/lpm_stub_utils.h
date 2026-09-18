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
 * \file lpm_stub_utils.h
 *
 * \brief Utility functions and macros shared across the LPM stub and its
 *        supporting modules (lpm_ddr, lpm_stub_main). Only compiled as
 *        part of the lpm_stub baremetal library.
 *
 */

#ifndef LPM_STUB_UTILS_H_
#define LPM_STUB_UTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* CTRL MMR partition unlock sequence.
 * Each partition is protected by a KICK register pair at:
 * base + (partition * LPM_STUB_CTRL_MMR_PARTITION_STRIDE) + LPM_STUB_CTRL_MMR_LOCK_CTRL_OFFSET + KICK_OFFSET
 */
#define LPM_STUB_CTRL_MMR_LOCKn_KICK0_OFFSET                            (8U)
#define LPM_STUB_CTRL_MMR_LOCKn_KICK1_OFFSET                            (0xCU)
#define LPM_STUB_CTRL_MMR_PARTITION_STRIDE                              (0x4000U)
#define LPM_STUB_CTRL_MMR_LOCK_CTRL_OFFSET                              (0x1000U)

/* MMR partition lock/unlock key values */
#define LPM_STUB_CTRL_MMR_KICK0_UNLOCK_VAL                              (0x68EF3490U)
#define LPM_STUB_CTRL_MMR_KICK1_UNLOCK_VAL                              (0xD172BC5AU)

/* CP15 DCCSW/DCISW register field positions (ARM architecture-defined):
 * Set field starts at bit: log2(cache_line_size_in_bytes) = log2(16) = 4.
 * Way field starts at bit: 32 - log2(num_ways) = 32 - log2(4) = 30.
 */
#define LPM_STUB_R5F_CP15_CACHE_SET_SHIFT                               (4U)
#define LPM_STUB_R5F_CP15_CACHE_WAY_SHIFT                               (30U)

/* R5F data cache geometry */
#define LPM_STUB_R5F_CACHE_NUM_WAYS                                     (0x4U)
#define LPM_STUB_R5F_CACHE_NUM_SETS                                     (0x200U)

/* Number of bootstrap IO pads that must remain accessible
 * on J784S4/J742S2 so the BootROM can detect the boot device
 * during resume.
 */
#define LPM_STUB_NUM_BOOTSTRAP_IOS                                      (18U)

/* Status value indicating the wake-daisy-chain controller has
 * latched the magic word and the hardware is ready to proceed
 */
#define LPM_STUB_WAKE_CTRL_STAT_LATCHED                                 (0x01U)

/* Non-zero initial value for the Lpm_stubHangAbort spin variable */
#define LPM_STUB_HANG_INIT_VAL                                          (0xAU)

/* TIMEOUT macro used in the LPM stub during polling loops */
#define LPM_STUB_TIMEOUT_MS                                             (100000U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Unlock a CTRL MMR partition so that its protected registers can be
 *        written.  Writes the standard two-step KICK sequence to the partition
 *        lock registers.
 *
 * \param base       Base address of the CTRL MMR instance
 *                   (e.g. CSL_WKUP_CTRL_MMR0_CFG0_BASE).
 * \param partition  Partition number to unlock (e.g. BOARD_MMR_PARTITION2).
 */
void Lpm_stubCtrlMmrUnlock(uint32_t base, uint32_t partition);

/** \brief Spin forever on an unrecoverable error; called when a timeout or
 *         hardware fault makes it unsafe to continue the retention sequence.
 */
void Lpm_stubHangAbort(void);

#endif /* LPM_STUB_UTILS_H_ */
