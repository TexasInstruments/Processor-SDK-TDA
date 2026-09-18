/*
 *  Copyright (c) Texas Instruments Incorporated 2023
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
 * \file sciclient_s2r.h
 *
 * \brief This file contains the declaration of the function to load and jump to
 * the suspend sequence.
 *
 */
#ifndef SCICLIENT_S2R_H_
#define SCICLIENT_S2R_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/**
 * Debug messages for suspend can be enabled by uncommenting the following macro
 * and recompiling
 */
// #define DEBUG_SUSPEND
#ifdef DEBUG_SUSPEND
#include <ti/drv/uart/UART_stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#ifdef DEBUG_SUSPEND
#define Sciclient_debugPrintf UART_printf
#else
#define Sciclient_debugPrintf(...) {};
#endif

/* MCU_SCRATCHPAD refers to MCU_PSRAM0_RAM in MCU Domain memory map */
#define SCICLIENT_MCU_SCRATCHPAD        (0x40280000U)
#define SCICLIENT_WAKE_REASON_OFFSET    (0x0U)

/* Low power mode wakeup sources */
#define SCICLIENT_LPM_WAKE_SOURCE_PMIC_GPIO (0x91U)
#define SCICLIENT_LPM_WAKE_SOURCE_INVALID   (0xFFFFFFFFU)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \struct Sciclient_LpmHeader
 *
 * \brief Lpm substructure header
 *
 * The structure is the header for the LPM part of PM-Boardcfg
 *
 * \param magic
 * Unique value indicating this is the LPM part of PM-Boardcfg
 *
 * \param size
 * Size of the entire LPM substructure including the size of this header
 *
 */
typedef struct {
    uint16_t    magic;
    uint16_t    size;
} __attribute__((__packed__))  Sciclient_LpmHeader;

/**
 * \struct Sciclient_LpmData
 *
 * \brief Lpm substructure extracted from the PM boardcfg
 *
 * The structure represents the LPM config part of the PM boardcfg.
 *
 * \param lpm_subhdr
 * Sub-header for the LPM part of the PM-Boardcfg
 *
 * \param suspend_initiator
 * Host ID of the suspend initiator
 *
 * \param lpm_mode
 * Low power mode that the system will enter into
 *
 * \param lpm_rsvd1
 * Reserved for future use
 *
 * \param lpm_rsvd2
 * Reserved for future use
 */
typedef struct {
    Sciclient_LpmHeader  lpm_subhdr;
    uint8_t              suspend_initiator;
    uint8_t              lpm_mode;
    uint16_t             lpm_rsvd1;
    uint32_t             lpm_rsvd2;
} __attribute__((packed)) Sciclient_LpmData;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Sciclient_goRetention Copy the suspend sequence to SRAM and jump to it.
 *
 *  \param  mode      Low power mode that the system is entering into
 *
 *  \return None
 */
void Sciclient_goRetention(uint32_t mode);

/**
 *  \brief Sciclient_s2rEnableWkupI2c Force WKUP_I2C0 device state to ON before
 *         entering an LPM sequence.
 *
 *  The LPM entry sequence communicates with the PMIC over WKUP_I2C0 to trigger
 *  IO and DDR retention. This call ensures the device is explicitly kept ON so
 *  that the lpm stub can use it to send commands to the PMIC.
 *
 *  \return CSL_PASS on success, else failure
 */
int32_t Sciclient_s2rEnableWkupI2c(void);

/**
 *  \brief Sciclient_getWakeReason Get the reason for wakeup from a low power mode
 *
 *  \param  msg       Pointer to TISCI request
 *
 *  \return CSL_PASS on success, else failure
 */
int32_t Sciclient_getWakeReason(uint32_t *msg);

/**
 *  \brief Sciclient_getSuspendMaster get the suspend master if it has been set before
 *
 *  \param  msg      Pointer to the tisci message
 *
 *  \return CSL_PASS on success, else failure
 */
int32_t Sciclient_getSuspendMaster(uint32_t *msg);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* SCICLIENT_S2R_H_ */
