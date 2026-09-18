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
 * \file lpm_mcu_only_recovery.h
 *
 * \brief API for switching the SoC between ACTIVE and MCU-only power states.
 *        Provides functions to transition the main domain on/off via PMIC state
 *        changes and domain isolation control.
 *
 */

#ifndef LPM_MCU_ONLY_RECOVERY_H_
#define LPM_MCU_ONLY_RECOVERY_H_

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

/* VTM temperature sensor control register addresses (WKUP_VTM_TMPSENS_CTRL_1..4) */
#define LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_1                        (0x42050320)
#define LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_2                        (0x42050340)
#define LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_3                        (0x42050360)
#define LPM_MCU_ONLY_WKUP_VTM_TMPSENS_CTRL_4                        (0x42050380)

/* WKUP_VTM_TMPSENS_CTRL — MAXT_OUTRG_EN field (bit 11) */
#define LPM_MCU_ONLY_MAXT_OUTRG_EN_MASK                             (0x00000800U)
#define LPM_MCU_ONLY_MAXT_OUTRG_EN_SHIFT                            (0x0000000BU)

/* WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL */

#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL_MAIN_VD_OFF_MASK       (0x00000001U)
#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL_MAIN_VD_OFF_SHIFT      (0x00000000U)
#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL_MAIN_VD_OFF_EN         (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_VDOM_CTRL_MAIN_VD_OFF_DIS        (0x0U)

/* Addresses used for testing main domain accesses after
 * it has been recovered.
 */
#define LPM_MCU_ONLY_DDR_TEST_ADDRESS                                 (0xA0000000)
#define LPM_MCU_ONLY_DDR_TEST_VAL                                     (0xDEADBEEF)
#define LPM_MCU_ONLY_MSMC_TEST_ADDRESS                                (0x70000000)
#define LPM_MCU_ONLY_MSMC_TEST_VAL                                    (0xC0DEC0DE)
#define LPM_MCU_ONLY_MCAN_REV_REG                                     (0x02700000)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Disable the VTM max-outrange alert for temperature sensors 1 through 4
 *        in the MAIN domain.
 *
 * Clears the MAXT_OUTRG_EN bit in the VTM TMPSENS_CTRL registers for
 * TMPSENS1..TMPSENS4. This must be called before entering MCU-only mode to
 * prevent spurious thermal alerts from firing while the MAIN domain is
 * powered down.
 */
void Lpm_mcuOnlyDisableMaxOutrgAlert(void);

/**
 * \brief Transition the SoC from ACTIVE state to MCU-only state.
 *
 * Issues a software reset to the MAIN domain, enables MAIN domain deep-sleep
 * isolation, then requests the PMIC to transition from ACTIVE to MCU-only
 * power state. After this call the MAIN domain is powered off and only the
 * MCU domain remains active.
 *
 * \return 0 on success.
 */
uint32_t Lpm_mcuOnlyActiveToMcuSwitch(void);

/**
 * \brief Transition the SoC from MCU-only state back to ACTIVE state.
 *
 * Requests the PMIC to transition from MCU-only to ACTIVE power state,
 * disables MAIN domain deep-sleep isolation, then re-enables the MAIN
 * domain so that its modules become operational again.
 *
 */
void Lpm_mcuOnlyToActiveSwitch(void);

#ifdef __cplusplus
}
#endif

#endif /* LPM_MCU_ONLY_RECOVERY_H_ */