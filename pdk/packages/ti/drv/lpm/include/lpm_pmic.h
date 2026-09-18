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
 *  \file lpm_pmic.h
 *
 *  \brief PMIC APIs and macros used during Low Power Modes.
 */

#ifndef LPM_PMIC_H_
#define LPM_PMIC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* PMIC GPIO configuration registers */
#define LPM_PMIC_GPIO2_CONF                            (0x00000032U)
#define LPM_PMIC_GPIO3_CONF                            (0x00000033U)
#define LPM_PMIC_GPIO4_CONF                            (0x00000034U)
#define LPM_PMIC_GPIO5_CONF                            (0x00000035U)
#define LPM_PMIC_GPIO6_CONF                            (0x00000036U)
#define LPM_PMIC_GPIO_OUT_1                            (0x0000003DU)
#define LPM_PMIC_GPIO_IN_1                             (0x0000003FU)

/* PMIC GPIO interrupt mask registers */
#define LPM_PMIC_MASK_GPIO1_8_FALL                     (0x0000004FU)
#define LPM_PMIC_MASK_GPIO1_8_RISE                     (0x00000050U)
#define LPM_PMIC_MASK_GPIO9_11                         (0x00000051U)

/* PMIC interrupt registers */
#define LPM_PMIC_INT_TOP                               (0x0000005AU)
#define LPM_PMIC_INT_BUCK                              (0x0000005BU)
#define LPM_PMIC_INT_BUCK1_2                           (0x0000005CU)
#define LPM_PMIC_INT_BUCK3_4                           (0x0000005DU)
#define LPM_PMIC_INT_BUCK5                             (0x0000005EU)
#define LPM_PMIC_INT_LDO_VMON                          (0x0000005FU)
#define LPM_PMIC_INT_LDO1_2                            (0x00000060U)
#define LPM_PMIC_INT_LDO3_4                            (0x00000061U)
#define LPM_PMIC_INT_VMON                              (0x00000062U)
#define LPM_PMIC_INT_GPIO                              (0x00000063U)
#define LPM_PMIC_INT_GPIO1_8                           (0x00000064U)
#define LPM_PMIC_INT_STARTUP                           (0x00000065U)
#define LPM_PMIC_INT_MISC                              (0x00000066U)
#define LPM_PMIC_INT_MODERATE_ERR                      (0x00000067U)
#define LPM_PMIC_INT_SEVERE_ERR                        (0x00000068U)
#define LPM_PMIC_INT_FSM_ERR                           (0x00000069U)
#define LPM_PMIC_INT_COMM_ERR                          (0x0000006AU)
#define LPM_PMIC_INT_READBACK_ERR                      (0x0000006BU)
#define LPM_PMIC_INT_ESM_ERR                           (0x0000006CU)

/* PMIC configuration registers */
#define LPM_PMIC_CONFIG_1                              (0x0000007DU)

/* PMIC FSM trigger registers */
#define LPM_PMIC_FSM_I2C_TRIGGERS                      (0x00000085U)
#define LPM_PMIC_FSM_NSLEEP_TRIGGERS                   (0x00000086U)

/* PMIC register lock, we must write unlock to this register to
 * enable writes to otherwise protected PMIC configuration registers
 */
#define LPM_PMIC_REGISTER_LOCK                         (0x000000A1U)

/* PMIC scratch pad register */
#define LPM_PMIC_SCRATCH_PAD_REG_3                     (0x000000CBU)

/* PMIC_GPIOx_CONF (common field layout for all GPIO_n_CONF registers) */

#define LPM_PMIC_GPIOx_CONF_GPIO_DIR_MASK              (0x00000001U)
#define LPM_PMIC_GPIOx_CONF_GPIO_DIR_SHIFT             (0x00000000U)

#define LPM_PMIC_GPIOx_CONF_GPIO_OD_MASK               (0x00000002U)
#define LPM_PMIC_GPIOx_CONF_GPIO_OD_SHIFT              (0x00000001U)

#define LPM_PMIC_GPIOx_CONF_GPIO_PU_SEL_MASK           (0x00000004U)
#define LPM_PMIC_GPIOx_CONF_GPIO_PU_SEL_SHIFT          (0x00000002U)

#define LPM_PMIC_GPIOx_CONF_GPIO_PU_PD_EN_MASK         (0x00000008U)
#define LPM_PMIC_GPIOx_CONF_GPIO_PU_PD_EN_SHIFT        (0x00000003U)

#define LPM_PMIC_GPIOx_CONF_GPIO_SEL_MASK              (0x000000E0U)
#define LPM_PMIC_GPIOx_CONF_GPIO_SEL_SHIFT             (0x00000005U)
#define LPM_PMIC_GPIOx_CONF_GPIO_SEL_LP_WKUP1          (0x000000C0U)

/* PMIC_GPIO_OUT_1 */

#define LPM_PMIC_GPIO_OUT_1_DDR_RET_VAL_MASK           (0x00000002U)
#define LPM_PMIC_GPIO_OUT_1_DDR_RET_VAL_SHIFT          (0x00000001U)

#define LPM_PMIC_GPIO_OUT_1_DDR_RET_CLK_MASK           (0x00000004U)
#define LPM_PMIC_GPIO_OUT_1_DDR_RET_CLK_SHIFT          (0x00000002U)

#define LPM_PMIC_GPIO_OUT_1_GPIO3_MASK                 (0x00000004U)
#define LPM_PMIC_GPIO_OUT_1_GPIO3_SHIFT                (0x00000002U)

#define LPM_PMIC_GPIO_OUT_1_GPIO4_MASK                 (0x00000008U)
#define LPM_PMIC_GPIO_OUT_1_GPIO4_SHIFT                (0x00000003U)

#define LPM_PMIC_GPIO_OUT_1_EN_GPIO_RET_3V3_MASK       (0x00000010U)
#define LPM_PMIC_GPIO_OUT_1_EN_GPIO_RET_3V3_SHIFT      (0x00000004U)

#define LPM_PMIC_GPIO_OUT_1_EN_DDR_RET_1V1_MASK        (0x00000020U)
#define LPM_PMIC_GPIO_OUT_1_EN_DDR_RET_1V1_SHIFT       (0x00000005U)

/* PMIC_INT_TOP */

#define LPM_PMIC_INT_TOP_BUCK_MASK                     (0x00000001U)
#define LPM_PMIC_INT_TOP_BUCK_SHIFT                    (0x00000000U)

#define LPM_PMIC_INT_TOP_LDO_VMON_MASK                 (0x00000002U)
#define LPM_PMIC_INT_TOP_LDO_VMON_SHIFT                (0x00000001U)

#define LPM_PMIC_INT_TOP_GPIO_MASK                     (0x00000004U)
#define LPM_PMIC_INT_TOP_GPIO_SHIFT                    (0x00000002U)

#define LPM_PMIC_INT_TOP_STARTUP_MASK                  (0x00000008U)
#define LPM_PMIC_INT_TOP_STARTUP_SHIFT                 (0x00000003U)

#define LPM_PMIC_INT_TOP_MISC_MASK                     (0x00000010U)
#define LPM_PMIC_INT_TOP_MISC_SHIFT                    (0x00000004U)

#define LPM_PMIC_INT_TOP_MODERATE_ERR_MASK             (0x00000020U)
#define LPM_PMIC_INT_TOP_MODERATE_ERR_SHIFT            (0x00000005U)

#define LPM_PMIC_INT_TOP_SEVERE_ERR_MASK               (0x00000040U)
#define LPM_PMIC_INT_TOP_SEVERE_ERR_SHIFT              (0x00000006U)

#define LPM_PMIC_INT_TOP_FSM_ERR_MASK                  (0x00000080U)
#define LPM_PMIC_INT_TOP_FSM_ERR_SHIFT                 (0x00000007U)

/* PMIC_INT_BUCK */

#define LPM_PMIC_INT_BUCK_BUCK1_2_MASK                 (0x00000001U)
#define LPM_PMIC_INT_BUCK_BUCK1_2_SHIFT                (0x00000000U)

#define LPM_PMIC_INT_BUCK_BUCK3_4_MASK                 (0x00000002U)
#define LPM_PMIC_INT_BUCK_BUCK3_4_SHIFT                (0x00000001U)

#define LPM_PMIC_INT_BUCK_BUCK5_MASK                   (0x00000004U)
#define LPM_PMIC_INT_BUCK_BUCK5_SHIFT                  (0x00000002U)

/* PMIC_INT_LDO_VMON */

#define LPM_PMIC_INT_LDO_VMON_LDO1_2_MASK              (0x00000001U)
#define LPM_PMIC_INT_LDO_VMON_LDO1_2_SHIFT             (0x00000000U)

#define LPM_PMIC_INT_LDO_VMON_LDO3_4_MASK              (0x00000002U)
#define LPM_PMIC_INT_LDO_VMON_LDO3_4_SHIFT             (0x00000001U)

#define LPM_PMIC_INT_LDO_VMON_VMON_MASK                (0x00000004U)
#define LPM_PMIC_INT_LDO_VMON_VMON_SHIFT               (0x00000002U)

/* PMIC_INT_GPIO */

#define LPM_PMIC_INT_GPIO_GPIO2_INT_MASK               (0x00000002U)
#define LPM_PMIC_INT_GPIO_GPIO2_INT_SHIFT              (0x00000001U)

#define LPM_PMIC_INT_GPIO_GPIO1_8_MASK                 (0x00000008U)
#define LPM_PMIC_INT_GPIO_GPIO1_8_SHIFT                (0x00000003U)

/* PMIC_INT_GPIO1_8 */

#define LPM_PMIC_INT_GPIO1_8_GPIO4_INT_MASK            (0x00000008U)
#define LPM_PMIC_INT_GPIO1_8_GPIO4_INT_SHIFT           (0x00000003U)

#define LPM_PMIC_INT_GPIO1_8_GPIO7_INT_MASK            (0x00000040U)
#define LPM_PMIC_INT_GPIO1_8_GPIO7_INT_SHIFT           (0x00000006U)

#define LPM_PMIC_INT_GPIO1_8_GPIO8_INT_MASK            (0x00000080U)
#define LPM_PMIC_INT_GPIO1_8_GPIO8_INT_SHIFT           (0x00000007U)

/* Combined mask for all GPIO wake interrupt bits cleared during MCU Only -> Active transition */
#define LPM_PMIC_INT_GPIO1_8_WAKE_INT_MASK             (LPM_PMIC_INT_GPIO1_8_GPIO4_INT_MASK | \
                                                        LPM_PMIC_INT_GPIO1_8_GPIO7_INT_MASK | \
                                                        LPM_PMIC_INT_GPIO1_8_GPIO8_INT_MASK)

/* PMIC_INT_STARTUP */

#define LPM_PMIC_INT_STARTUP_NPWRON_START_MASK         (0x00000002U)
#define LPM_PMIC_INT_STARTUP_NPWRON_START_SHIFT        (0x00000001U)

/* PMIC_INT_FSM_ERR */

#define LPM_PMIC_INT_FSM_ERR_COMM_ERR_MASK             (0x00000010U)
#define LPM_PMIC_INT_FSM_ERR_COMM_ERR_SHIFT            (0x00000004U)

#define LPM_PMIC_INT_FSM_ERR_READBACK_ERR_MASK         (0x00000020U)
#define LPM_PMIC_INT_FSM_ERR_READBACK_ERR_SHIFT        (0x00000005U)

#define LPM_PMIC_INT_FSM_ERR_ESM_MASK                  (0x00000040U)
#define LPM_PMIC_INT_FSM_ERR_ESM_SHIFT                 (0x00000006U)

/* PMIC_CONFIG_1 */

#define LPM_PMIC_CONFIG_1_NSLEEP_MASK                  (0x000000C0U)
#define LPM_PMIC_CONFIG_1_NSLEEP_SHIFT                 (0x00000006U)

/* PMIC_FSM_I2C_TRIGGERS */

#define LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_5_MASK  (0x00000020U)
#define LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_5_SHIFT (0x00000005U)

#define LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK  (0x00000040U)
#define LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_SHIFT (0x00000006U)

#define LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_MASK   (0x00000080U)
#define LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_SHIFT  (0x00000007U)

/* PMIC FSM value for DDR retention */
#define LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET              (0x80U)

/* PMIC GPIO masks */
#define LPM_PMIC_MASK_GPIO4_ONLY                       (0x000000F7U)
#define LPM_PMIC_MASK_ALL_GPIO1_8                      (0x000000FFU)
#define LPM_PMIC_MASK_ALL_GPIO9_11                     (0x0000003FU)

/* PMIC FSM states */
#define LPM_PMIC_ACTIVE_STATE                          (0x03U)
#define LPM_PMIC_MCU_ONLY_STATE                        (0x02U)
#define LPM_PMIC_RETENTION_STATE                       (0x00U)

/* Low power modes */
#define LPM_PMIC_SOC_OFF                               (0x5U)
#define LPM_PMIC_IO_ONLY_PLUS_DDR                      (0x2U)

/* PMIC unlock value used to make configuration registers accessible */
#define LPM_PMIC_REGISTER_LOCK_UNLOCK                  (0x0000009BU)

/* PMIC magic word used for resume detection by the bootloader */
#define LPM_PMIC_SCRATCH_PAD_REG_3_MAGIC_SUSPEND       (0x000000BAU)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Top level PMIC initialization function for LPM.
 */
uint32_t Lpm_pmicInit(void);

/**
 * \brief Configure the PMIC to power down the SOC for the given low power mode in the lpm stub.
 *
 * \param lpm_mode  Low power mode to enter (LPM_PMIC_SOC_OFF or LPM_PMIC_IO_ONLY_PLUS_DDR).
 */
void Lpm_pmicSetupForLpm(uint32_t lpm_mode);

/**
 * \brief Transition the PMIC from MCU Only state back to Active state.
 *
 * Masks the NSLEEP pins, asserts ACTIVE state on FSM_NSLEEP_TRIGGERS, then
 * un-masks the pins and waits for the state change to complete.
 */
void Lpm_pmicStateChangeMCUOnlyToActive(void);

/**
 * \brief Transition the PMIC from Active state to MCU Only state.
 *
 * Updates FSM_NSLEEP_TRIGGERS and clears pending interrupts to allow the
 * PMIC to enter MCU Only mode.
 */
void Lpm_pmicStateChangeActiveToMCUOnly(void);

/**
 * \brief Transition the PMIC from Active state to IO Retention state.
 *
 * Configures PMIC to enter IO Retention mode,
 * keeping MCU domain and DDR powered while isolating MAIN domain IOs.
 */
void Lpm_pmicStateChangeActiveToIORetention(void);

#ifdef __cplusplus
}
#endif

#endif /* LPM_PMIC_H_ */
