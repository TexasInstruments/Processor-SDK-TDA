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
 * \file lpm_i2c_baremetal.h
 *
 * \brief Bare-metal I2C APIs used to configure the PMIC in LPM stub.
 *
 */

#ifndef LPM_I2C_BAREMETAL_H_
#define LPM_I2C_BAREMETAL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

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

/* Own address written to the I2C_OA register; required by the hardware even
 * in master-only use so the address comparator is initialised
 */
#define LPM_I2C_OA_REG                           (0xB4U)

/* Prescaler divider value for 96 MHz input: 96 / (7+1) = 12 MHz internal clock */
#define LPM_I2C_PRESCALER_DIVIDER_12MHZ          (0x7U)

/* SCLL register value for 400 kHz Fast Mode; total low period = 9+7 = 16 cycles at 12 MHz = 1.333 μs */
#define LPM_I2C_SCLL_16                          (0x9U)

/* SCLH register value for 400 kHz Fast Mode; total high period = 9+5 = 14 cycles at 12 MHz = 1.167 μs */
#define LPM_I2C_SCLH_14                          (0x9U)

/* Reserved bit 14 of CON register that must be cleared during reconfiguration */
#define LPM_I2C_CON_RESERVED_MASK                (0x00004000U)

#define LPM_I2C_DETECT_TIMEOUT                   (500U)

/* Combined mask of all CON fields that control an active transfer; used to
 * clear the controller state before re-configuring for the next transaction
 */
#define LPM_I2C_CON_WRITE_MASK                   (CSL_I2C_CON_STT_MASK      | \
                                                  CSL_I2C_CON_STP_MASK      | \
                                                  CSL_I2C_CON_XSA_MASK      | \
                                                  CSL_I2C_CON_TRX_MASK      | \
                                                  CSL_I2C_CON_MST_MASK      | \
                                                  CSL_I2C_CON_STB_MASK      | \
                                                  LPM_I2C_CON_RESERVED_MASK | \
                                                  CSL_I2C_CON_I2C_EN_MASK)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief  Read one byte from the I2C slave register at address \a add.
 *         Blocks indefinitely (timeout = 0xFFFFFFFF).
 *
 * \param  add  Register address to read.
 *
 * \return Byte value read from the register.
 */
uint8_t Lpm_i2cRead(char add);

/**
 * \brief  Write one byte \a data to the I2C slave register at address \a add.
 *         Blocks until the transfer completes.
 *
 * \param  add   Register address to write.
 * \param  data  Byte value to write.
 */
void Lpm_i2cWrite(char add, char data);

/**
 * \brief  Configure the WKUP I2C0 controller to access the PMIC
 *         During low power mode configuration.
 *
 * \param  pmic  I2C slave address of the target PMIC.
 */
void Lpm_i2cConfigWkup(char pmic);

#ifdef __cplusplus
}
#endif

#endif /* LPM_I2C_BAREMETAL_H_ */
