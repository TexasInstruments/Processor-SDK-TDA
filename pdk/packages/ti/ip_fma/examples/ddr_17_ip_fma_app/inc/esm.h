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
 *  \file     esm.h
 *
 *  \brief    This file contains defines and declarations for ESM(Error Signaling Module)
 */

/*===========================================================================*/
/*                            Include files                                  */
/*===========================================================================*/

#ifndef DDR_T7_ESM_H
#define DDR_T7_ESM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/csl/soc.h>
#include <ti/csl/csl_esm.h>
#include <ti/csl/src/ip/ecc_aggr/V1/csl_ecc_aggr.h>

#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_intr_esm0.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_intr_esm0.h>
#endif

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

typedef struct CSL_esm_app_R5_cfg_s {
    uint32_t    hi_pri_evt;
    uint32_t    lo_pri_evt;
} CSL_esm_app_R5_cfg;

/* ESM Base Addresses */
#define CSL_TEST_ESM_EN_KEY_ENABLE_VAL      (0xFU)
#define NO_EVENT_VALUE                      (0xffffu)

#define ESM_CFG_BASE                        (CSL_ESM0_CFG_BASE)
#define ESM_LO_INT                          (CSLR_MCU_R5FSS0_CORE0_INTR_ESM0_ESM_INT_LOW_LVL_0)
#define ESM_HI_INT                          (CSLR_MCU_R5FSS0_CORE0_INTR_ESM0_ESM_INT_HI_LVL_0)
#define ESM_CFG_ERR_INT                     (CSLR_MCU_R5FSS0_CORE0_INTR_ESM0_ESM_INT_CFG_LVL_0)
#define ESM_MAX_EVENT_MAP_NUM_WORDS         (4u)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t cslAppEsmSetup(CSL_esm_app_R5_cfg* cfg);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef DDR_T7_ESM_H */

