/*
 *   Copyright (c) Texas Instruments Incorporated 2026
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
 *
 */

/**
 *  \file     csl_esm_asil.c
 *
 *  \brief    This file contains the implementation of the APIs present in the
 *            device abstraction layer file of ESM that safe modules require.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/hw_types.h>
#include <ti/csl/soc.h>
#include <ti/csl/csl_esm.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define ESM_NUM_INTR_PER_GRP              (32U)
#define ESM_MAX_NUM_INTRS                 (1024U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  Requirement: REQ_TAG(PDK-6028)
 *  Design: did_csl_handle_esm_cfg_err_int
 */
int32_t ESMClearIntrStatus(uint32_t baseAddr, uint32_t intrSrc)
{
    int32_t  retVal = CSL_EBADARGS;
    uint32_t regVal = 0U;
    uint32_t regOffset;
    uintptr_t regAddr;
    if ( (baseAddr != ((uint32_t) (0u))) &&
         (intrSrc  < ESM_MAX_NUM_INTRS))
    {
        regVal = ((uint32_t) 0x1U << (intrSrc % ESM_NUM_INTR_PER_GRP));
        regOffset = CSL_ESM_ERR_GRP_STS(intrSrc / ESM_NUM_INTR_PER_GRP);
        regAddr = (uintptr_t) baseAddr + (uintptr_t) regOffset;
        HW_WR_REG32(regAddr, regVal);
        retVal = CSL_PASS;
    }
    return retVal;
}

/**
 *  Requirement: REQ_TAG(PDK-6028)
 *  Design: did_csl_handle_esm_cfg_err_int
 */
int32_t ESMWriteEOI(uint32_t baseAddr, esmIntrType_t intrType)
{
    int32_t  retVal = CSL_EBADARGS;
    uintptr_t regAddr;
    if (baseAddr != ((uint32_t) (0u)))
    {
        regAddr = (uintptr_t) baseAddr + (uintptr_t) CSL_ESM_EOI;
        HW_WR_FIELD32(regAddr, CSL_ESM_EOI_KEY, intrType);
        retVal = CSL_PASS;
    }
    return retVal;
}
