/**
 *   Copyright (c) Texas Instruments Incorporated 2021-2022
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
 *  \file csl_hts.c
 *
 *  \brief File containing the VPAC HTS HAL init, deinit and other common
 *  functions.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <src/csl/include/csl_hts.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* All HTS register offset macros moved to csl_hts.h for external access */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* CSL_HtsHwaParams structure is now defined in csl_hts.h */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t HtsCheckConfig(const CSL_HtsSchConfig *cfg);

static void HtsSetSchConfig(uint32_t baseAddr, const CSL_HtsSchConfig *cfg);
static void HtsSchStart(uint32_t baseAddr, const CSL_HtsSchConfig *cfg);
static void HtsSchStop(uint32_t baseAddr, const CSL_HtsSchConfig *cfg);

static void HtsSetDmaSchConsConfig(uint32_t baseAddr,
    const CSL_HtsDmaConsConfig *cfg);
static void HtsDmaSchConsStart(uint32_t baseAddr,
    const CSL_HtsDmaConsConfig *cfg);
static void HtsDmaSchConsStop(uint32_t baseAddr,
    const CSL_HtsDmaConsConfig *cfg);

static void HtsSetDmaSchProdConfig(uint32_t baseAddr,
    const CSL_HtsDmaProdConfig *cfg,
    uint32_t isPaSupported);
static void HtsDmaSchProdStart(uint32_t baseAddr,
    const CSL_HtsDmaProdConfig *cfg, uint32_t isPaSupported);
static void HtsDmaSchProdStop(uint32_t baseAddr,
    const CSL_HtsDmaProdConfig *cfg, uint32_t isPaSupported);

static void HtsSetSchConsConfig(uint32_t baseAddr, const CSL_HtsConsConfig *cfg);
static void HtsSchConsStart(uint32_t baseAddr, const CSL_HtsConsConfig *cfg);
static void HtsSchConsStop(uint32_t baseAddr, const CSL_HtsConsConfig *cfg);

static void HtsSetSchProdConfig(uint32_t baseAddr, const CSL_HtsProdConfig *cfg,
    uint32_t isPaSupported);
static void HtsSchProdStart(uint32_t baseAddr, const CSL_HtsProdConfig *cfg,
    uint32_t isPaSupported);
static void HtsSchProdStop(uint32_t baseAddr, const CSL_HtsProdConfig *cfg,
    uint32_t isPaSupported);

static void HtsSetDynThldConfig(uint32_t baseAddr,
    const CSL_HtsDynThldConfig *cfg);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

const CSL_HtsHwaParams gCslHtsParams[CSL_HTS_HWA_MAX_SCH] =
{
    {
        CSL_HTS_HWA_SCH_VISS0,
        CSL_HTS_HWA0_SCHEDULER_CONTROL,
        CSL_HTS_NUM_VISS_CONSUMERS,
        {
            CSL_HTS_HWA0_CONS0_CONTROL,
            CSL_HTS_HWA0_CONS1_CONTROL,
            CSL_HTS_HWA0_CONS2_CONTROL,
            CSL_HTS_HWA0_CONS3_CONTROL,
            CSL_HTS_HWA0_CONS4_CONTROL,
            CSL_HTS_HWA0_CONS5_CONTROL
        },
        CSL_HTS_NUM_VISS_PRODUCERS,
        {
            CSL_HTS_HWA0_PROD0_CONTROL,
            CSL_HTS_HWA0_PROD1_CONTROL,
            CSL_HTS_HWA0_PROD2_CONTROL,
            CSL_HTS_HWA0_PROD3_CONTROL,
            CSL_HTS_HWA0_PROD4_CONTROL,
            CSL_HTS_HWA0_PROD5_CONTROL,
            CSL_HTS_HWA0_PROD6_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UTRUE, UTRUE, UTRUE, UTRUE, UFALSE, UFALSE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA0_SCHEDULER_CONTROL,
            CSL_HTS_DMA1_SCHEDULER_CONTROL,
            CSL_HTS_DMA2_SCHEDULER_CONTROL,
            CSL_HTS_DMA3_SCHEDULER_CONTROL,
            CSL_HTS_DMA4_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA240_SCHEDULER_CONTROL,
            CSL_HTS_DMA241_SCHEDULER_CONTROL,
            CSL_HTS_DMA242_SCHEDULER_CONTROL,
            CSL_HTS_DMA243_SCHEDULER_CONTROL,
            CSL_HTS_DMA244_SCHEDULER_CONTROL,
            CSL_HTS_DMA245_SCHEDULER_CONTROL,
            CSL_HTS_DMA245_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_VISS1,
        CSL_HTS_HWA1_SCHEDULER_CONTROL,
        3U,
        {
            CSL_HTS_HWA1_CONS0_CONTROL,
            CSL_HTS_HWA1_CONS1_CONTROL,
            CSL_HTS_HWA1_CONS2_CONTROL,
            CSL_HTS_HWA1_CONS3_CONTROL,
            CSL_HTS_HWA1_CONS4_CONTROL,
            CSL_HTS_HWA1_CONS5_CONTROL
        },
        6U,
        {
            CSL_HTS_HWA1_PROD0_CONTROL,
            CSL_HTS_HWA1_PROD1_CONTROL,
            CSL_HTS_HWA1_PROD2_CONTROL,
            CSL_HTS_HWA1_PROD3_CONTROL,
            CSL_HTS_HWA1_PROD4_CONTROL,
            CSL_HTS_HWA1_PROD5_CONTROL,
            CSL_HTS_HWA1_PROD6_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA8_SCHEDULER_CONTROL,
            CSL_HTS_DMA9_SCHEDULER_CONTROL,
            CSL_HTS_DMA10_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UTRUE, UTRUE, UTRUE, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA256_SCHEDULER_CONTROL,
            CSL_HTS_DMA257_SCHEDULER_CONTROL,
            CSL_HTS_DMA258_SCHEDULER_CONTROL,
            CSL_HTS_DMA259_SCHEDULER_CONTROL,
            CSL_HTS_DMA260_SCHEDULER_CONTROL,
            CSL_HTS_DMA261_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_LDC0,
        CSL_HTS_HWA2_SCHEDULER_CONTROL,
        CSL_HTS_NUM_LDC_CONSUMERS,
        {
            CSL_HTS_HWA2_CONS0_CONTROL,
            CSL_HTS_HWA2_CONS1_CONTROL,
            CSL_HTS_HWA2_CONS2_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        CSL_HTS_NUM_LDC_PRODUCERS,
        {
            CSL_HTS_HWA2_PROD0_CONTROL,
            CSL_HTS_HWA2_PROD1_CONTROL,
            CSL_HTS_HWA2_PROD2_CONTROL,
            CSL_HTS_HWA2_PROD3_CONTROL,
            CSL_HTS_HWA2_PROD4_CONTROL,
            CSL_HTS_HWA2_PROD5_CONTROL,
            CSL_HTS_HWA2_PROD6_CONTROL,
            CSL_HTS_HWA2_PROD6_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UTRUE, UTRUE, UTRUE, UTRUE, UTRUE, UTRUE, UTRUE, UTRUE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA8_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA272_SCHEDULER_CONTROL,
            CSL_HTS_DMA273_SCHEDULER_CONTROL,
            CSL_HTS_DMA274_SCHEDULER_CONTROL,
            CSL_HTS_DMA275_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_LDC1,
        CSL_HTS_HWA3_SCHEDULER_CONTROL,
        1U,
        {
            CSL_HTS_HWA3_CONS0_CONTROL,
            CSL_HTS_HWA3_CONS1_CONTROL,
            CSL_HTS_HWA3_CONS2_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA3_PROD0_CONTROL,
            CSL_HTS_HWA3_PROD1_CONTROL,
            CSL_HTS_HWA3_PROD2_CONTROL,
            CSL_HTS_HWA3_PROD3_CONTROL,
            CSL_HTS_HWA3_PROD4_CONTROL,
            CSL_HTS_HWA3_PROD5_CONTROL,
            CSL_HTS_HWA3_PROD6_CONTROL,
            CSL_HTS_HWA3_PROD7_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UTRUE, UTRUE, UTRUE, UTRUE, UTRUE, UTRUE, UTRUE, UTRUE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA8_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA288_SCHEDULER_CONTROL,
            CSL_HTS_DMA289_SCHEDULER_CONTROL,
            CSL_HTS_DMA290_SCHEDULER_CONTROL,
            CSL_HTS_DMA291_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_MSC0,
        CSL_HTS_HWA4_SCHEDULER_CONTROL,
        CSL_HTS_NUM_MSC0_CONSUMERS,
        {
            CSL_HTS_HWA4_CONS0_CONTROL,
            CSL_HTS_HWA4_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        CSL_HTS_NUM_MSC0_PRODUCERS,
        {
            CSL_HTS_HWA4_PROD0_CONTROL,
            CSL_HTS_HWA4_PROD1_CONTROL,
            CSL_HTS_HWA4_PROD2_CONTROL,
            CSL_HTS_HWA4_PROD3_CONTROL,
            CSL_HTS_HWA4_PROD4_CONTROL,
            CSL_HTS_HWA4_PROD5_CONTROL,
            CSL_HTS_HWA4_PROD6_CONTROL,
            CSL_HTS_HWA4_PROD7_CONTROL,
            CSL_HTS_HWA4_PROD8_CONTROL,
            CSL_HTS_HWA4_PROD9_CONTROL,
            CSL_HTS_HWA4_PROD10_CONTROL
        },
        {UTRUE, UTRUE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA32_SCHEDULER_CONTROL,
            CSL_HTS_DMA33_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UTRUE, UTRUE, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA304_SCHEDULER_CONTROL,
            CSL_HTS_DMA305_SCHEDULER_CONTROL,
            CSL_HTS_DMA306_SCHEDULER_CONTROL,
            CSL_HTS_DMA307_SCHEDULER_CONTROL,
            CSL_HTS_DMA308_SCHEDULER_CONTROL,
            CSL_HTS_DMA309_SCHEDULER_CONTROL,
            CSL_HTS_DMA310_SCHEDULER_CONTROL,
            CSL_HTS_DMA311_SCHEDULER_CONTROL,
            CSL_HTS_DMA312_SCHEDULER_CONTROL,
            CSL_HTS_DMA313_SCHEDULER_CONTROL,
            CSL_HTS_DMA313_SCHEDULER_CONTROL
        }
    },
    {
        CSL_HTS_HWA_SCH_MSC1,
        CSL_HTS_HWA5_SCHEDULER_CONTROL,
        CSL_HTS_NUM_MSC1_CONSUMERS,
        {
            CSL_HTS_HWA5_CONS0_CONTROL,
            CSL_HTS_HWA5_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        CSL_HTS_NUM_MSC1_PRODUCERS,
        {
            CSL_HTS_HWA5_PROD0_CONTROL,
            CSL_HTS_HWA5_PROD1_CONTROL,
            CSL_HTS_HWA5_PROD2_CONTROL,
            CSL_HTS_HWA5_PROD3_CONTROL,
            CSL_HTS_HWA5_PROD4_CONTROL,
            CSL_HTS_HWA5_PROD5_CONTROL,
            CSL_HTS_HWA5_PROD6_CONTROL,
            CSL_HTS_HWA5_PROD7_CONTROL,
            CSL_HTS_HWA5_PROD8_CONTROL,
            CSL_HTS_HWA5_PROD9_CONTROL,
            CSL_HTS_HWA5_PROD10_CONTROL
        },
        {UTRUE, UTRUE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA40_SCHEDULER_CONTROL,
            CSL_HTS_DMA41_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UTRUE, UTRUE, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA304_SCHEDULER_CONTROL,
            CSL_HTS_DMA305_SCHEDULER_CONTROL,
            CSL_HTS_DMA306_SCHEDULER_CONTROL,
            CSL_HTS_DMA307_SCHEDULER_CONTROL,
            CSL_HTS_DMA308_SCHEDULER_CONTROL,
            CSL_HTS_DMA309_SCHEDULER_CONTROL,
            CSL_HTS_DMA310_SCHEDULER_CONTROL,
            CSL_HTS_DMA311_SCHEDULER_CONTROL,
            CSL_HTS_DMA312_SCHEDULER_CONTROL,
            CSL_HTS_DMA313_SCHEDULER_CONTROL,
            CSL_HTS_DMA313_SCHEDULER_CONTROL
        }
    },
    {
        CSL_HTS_HWA_SCH_NF,
        CSL_HTS_HWA6_SCHEDULER_CONTROL,
        CSL_HTS_NUM_NF_CONSUMERS,
        {
            CSL_HTS_HWA6_CONS0_CONTROL,
            CSL_HTS_HWA6_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        CSL_HTS_NUM_NF_PRODUCERS,
        {
            CSL_HTS_HWA6_PROD0_CONTROL,
            CSL_HTS_HWA6_PROD1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA48_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA336_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_DOF,
        CSL_HTS_HWA0_SCHEDULER_CONTROL,
        5U,
        {
            CSL_HTS_HWA0_CONS0_CONTROL,
            CSL_HTS_HWA0_CONS1_CONTROL,
            CSL_HTS_HWA0_CONS2_CONTROL,
            CSL_HTS_HWA0_CONS3_CONTROL,
            CSL_HTS_HWA0_CONS4_CONTROL,
            CSL_HTS_HWA0_CONS5_CONTROL
        },
        6U,
        {
            CSL_HTS_HWA0_PROD0_CONTROL,
            CSL_HTS_HWA0_PROD1_CONTROL,
            CSL_HTS_HWA0_PROD2_CONTROL,
            CSL_HTS_HWA0_PROD3_CONTROL,
            CSL_HTS_HWA0_PROD4_CONTROL,
            CSL_HTS_HWA0_PROD5_CONTROL,
            CSL_HTS_HWA0_PROD6_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UFALSE, UFALSE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA0_SCHEDULER_CONTROL,
            CSL_HTS_DMA1_SCHEDULER_CONTROL,
            CSL_HTS_DMA2_SCHEDULER_CONTROL,
            CSL_HTS_DMA3_SCHEDULER_CONTROL,
            CSL_HTS_DMA4_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA240_SCHEDULER_CONTROL,
            CSL_HTS_DMA241_SCHEDULER_CONTROL,
            CSL_HTS_DMA242_SCHEDULER_CONTROL,
            CSL_HTS_DMA243_SCHEDULER_CONTROL,
            CSL_HTS_DMA244_SCHEDULER_CONTROL,
            CSL_HTS_DMA245_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_SDE,
        CSL_HTS_HWA1_SCHEDULER_CONTROL,
        3U,
        {
            CSL_HTS_HWA1_CONS0_CONTROL,
            CSL_HTS_HWA1_CONS1_CONTROL,
            CSL_HTS_HWA1_CONS2_CONTROL,
            CSL_HTS_HWA1_CONS3_CONTROL,
            CSL_HTS_HWA1_CONS4_CONTROL,
            CSL_HTS_HWA1_CONS5_CONTROL
        },
        6U,
        {
            CSL_HTS_HWA1_PROD0_CONTROL,
            CSL_HTS_HWA1_PROD1_CONTROL,
            CSL_HTS_HWA1_PROD2_CONTROL,
            CSL_HTS_HWA1_PROD3_CONTROL,
            CSL_HTS_HWA1_PROD4_CONTROL,
            CSL_HTS_HWA1_PROD5_CONTROL,
            CSL_HTS_HWA1_PROD6_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {   UTRUE, UTRUE, UFALSE, UFALSE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA8_SCHEDULER_CONTROL,
            CSL_HTS_DMA9_SCHEDULER_CONTROL,
            CSL_HTS_DMA10_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UTRUE, UTRUE, UTRUE, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA256_SCHEDULER_CONTROL,
            CSL_HTS_DMA257_SCHEDULER_CONTROL,
            CSL_HTS_DMA258_SCHEDULER_CONTROL,
            CSL_HTS_DMA259_SCHEDULER_CONTROL,
            CSL_HTS_DMA260_SCHEDULER_CONTROL,
            CSL_HTS_DMA261_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_FOCO1,
        CSL_HTS_HWA7_SCHEDULER_CONTROL,
        4U,
        {
            CSL_HTS_HWA7_CONS0_CONTROL,
            CSL_HTS_HWA7_CONS1_CONTROL,
            CSL_HTS_HWA7_CONS2_CONTROL,
            CSL_HTS_HWA7_CONS3_CONTROL,
            CSL_HTS_HWA7_CONS4_CONTROL,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA7_PROD0_CONTROL,
            CSL_HTS_HWA7_PROD1_CONTROL,
            CSL_HTS_HWA7_PROD2_CONTROL,
            CSL_HTS_HWA7_PROD3_CONTROL,
            CSL_HTS_HWA7_PROD4_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UFALSE, UFALSE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA56_SCHEDULER_CONTROL,
            CSL_HTS_DMA57_SCHEDULER_CONTROL,
            CSL_HTS_DMA58_SCHEDULER_CONTROL,
            CSL_HTS_DMA59_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA352_SCHEDULER_CONTROL,
            CSL_HTS_DMA353_SCHEDULER_CONTROL,
            CSL_HTS_DMA354_SCHEDULER_CONTROL,
            CSL_HTS_DMA355_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_FOCO2,
        CSL_HTS_HWA8_SCHEDULER_CONTROL,
        4U,
        {
            CSL_HTS_HWA8_CONS0_CONTROL,
            CSL_HTS_HWA8_CONS1_CONTROL,
            CSL_HTS_HWA8_CONS2_CONTROL,
            CSL_HTS_HWA8_CONS3_CONTROL,
            CSL_HTS_HWA8_CONS4_CONTROL,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA8_PROD0_CONTROL,
            CSL_HTS_HWA8_PROD1_CONTROL,
            CSL_HTS_HWA8_PROD2_CONTROL,
            CSL_HTS_HWA8_PROD3_CONTROL,
            CSL_HTS_HWA8_PROD4_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UFALSE, UFALSE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA64_SCHEDULER_CONTROL,
            CSL_HTS_DMA65_SCHEDULER_CONTROL,
            CSL_HTS_DMA66_SCHEDULER_CONTROL,
            CSL_HTS_DMA67_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA368_SCHEDULER_CONTROL,
            CSL_HTS_DMA369_SCHEDULER_CONTROL,
            CSL_HTS_DMA370_SCHEDULER_CONTROL,
            CSL_HTS_DMA371_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_SP0,
        CSL_HTS_HWA12_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA12_CONS0_CONTROL,
            CSL_HTS_HWA12_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA12_PROD0_CONTROL,
            CSL_HTS_HWA12_PROD1_CONTROL,
            CSL_HTS_HWA12_PROD2_CONTROL,
            CSL_HTS_HWA12_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA64_SCHEDULER_CONTROL,
            CSL_HTS_DMA65_SCHEDULER_CONTROL,
            CSL_HTS_DMA66_SCHEDULER_CONTROL,
            CSL_HTS_DMA67_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA368_SCHEDULER_CONTROL,
            CSL_HTS_DMA369_SCHEDULER_CONTROL,
            CSL_HTS_DMA370_SCHEDULER_CONTROL,
            CSL_HTS_DMA371_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_SP1,
        CSL_HTS_HWA13_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA13_CONS0_CONTROL,
            CSL_HTS_HWA13_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA13_PROD0_CONTROL,
            CSL_HTS_HWA13_PROD1_CONTROL,
            CSL_HTS_HWA13_PROD2_CONTROL,
            CSL_HTS_HWA13_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            CSL_HTS_DMA64_SCHEDULER_CONTROL,
            CSL_HTS_DMA65_SCHEDULER_CONTROL,
            CSL_HTS_DMA66_SCHEDULER_CONTROL,
            CSL_HTS_DMA67_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {
            CSL_HTS_DMA368_SCHEDULER_CONTROL,
            CSL_HTS_DMA369_SCHEDULER_CONTROL,
            CSL_HTS_DMA370_SCHEDULER_CONTROL,
            CSL_HTS_DMA371_SCHEDULER_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        }
    },
    {
        CSL_HTS_HWA_SCH_SP2,
        CSL_HTS_HWA14_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA14_CONS0_CONTROL,
            CSL_HTS_HWA14_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA14_PROD0_CONTROL,
            CSL_HTS_HWA14_PROD1_CONTROL,
            CSL_HTS_HWA14_PROD2_CONTROL,
            CSL_HTS_HWA14_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP3,
        CSL_HTS_HWA15_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA15_CONS0_CONTROL,
            CSL_HTS_HWA15_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA15_PROD0_CONTROL,
            CSL_HTS_HWA15_PROD1_CONTROL,
            CSL_HTS_HWA15_PROD2_CONTROL,
            CSL_HTS_HWA15_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP4,
        CSL_HTS_HWA16_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA16_CONS0_CONTROL,
            CSL_HTS_HWA16_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA16_PROD0_CONTROL,
            CSL_HTS_HWA16_PROD1_CONTROL,
            CSL_HTS_HWA16_PROD2_CONTROL,
            CSL_HTS_HWA16_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP5,
        CSL_HTS_HWA17_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA17_CONS0_CONTROL,
            CSL_HTS_HWA17_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA17_PROD0_CONTROL,
            CSL_HTS_HWA17_PROD1_CONTROL,
            CSL_HTS_HWA17_PROD2_CONTROL,
            CSL_HTS_HWA17_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP6,
        CSL_HTS_HWA18_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA18_CONS0_CONTROL,
            CSL_HTS_HWA18_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA18_PROD0_CONTROL,
            CSL_HTS_HWA18_PROD1_CONTROL,
            CSL_HTS_HWA18_PROD2_CONTROL,
            CSL_HTS_HWA18_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP7,
        CSL_HTS_HWA19_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA19_CONS0_CONTROL,
            CSL_HTS_HWA19_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA19_PROD0_CONTROL,
            CSL_HTS_HWA19_PROD1_CONTROL,
            CSL_HTS_HWA19_PROD2_CONTROL,
            CSL_HTS_HWA19_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP8,
        CSL_HTS_HWA20_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA20_CONS0_CONTROL,
            CSL_HTS_HWA20_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA20_PROD0_CONTROL,
            CSL_HTS_HWA20_PROD1_CONTROL,
            CSL_HTS_HWA20_PROD2_CONTROL,
            CSL_HTS_HWA20_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP9,
        CSL_HTS_HWA21_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA21_CONS0_CONTROL,
            CSL_HTS_HWA21_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA21_PROD0_CONTROL,
            CSL_HTS_HWA21_PROD1_CONTROL,
            CSL_HTS_HWA21_PROD2_CONTROL,
            CSL_HTS_HWA21_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        }
    },
    {
        CSL_HTS_HWA_SCH_SP14,
        CSL_HTS_HWA26_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA26_CONS0_CONTROL,
            CSL_HTS_HWA26_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA26_PROD0_CONTROL,
            CSL_HTS_HWA26_PROD1_CONTROL,
            CSL_HTS_HWA26_PROD2_CONTROL,
            CSL_HTS_HWA26_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        },
        CSL_HTS_HWA26_DYNAMIC_THRESHOLD0
    },
    {
        CSL_HTS_HWA_SCH_SP15,
        CSL_HTS_HWA27_SCHEDULER_CONTROL,
        2U,
        {
            CSL_HTS_HWA27_CONS0_CONTROL,
            CSL_HTS_HWA27_CONS1_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        4U,
        {
            CSL_HTS_HWA27_PROD0_CONTROL,
            CSL_HTS_HWA27_PROD1_CONTROL,
            CSL_HTS_HWA27_PROD2_CONTROL,
            CSL_HTS_HWA27_PROD3_CONTROL,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {
            UTRUE, UTRUE, UTRUE, UTRUE, UFALSE,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM,
            CSL_HTS_INVALID_PARAM
        },
        {

        },
        {UFALSE, UFALSE, UFALSE, UFALSE, UFALSE, CSL_HTS_INVALID_PARAM},
        {

        },
        CSL_HTS_HWA27_DYNAMIC_THRESHOLD0
    },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* Function to start the pipeline */
int32_t CSL_htsPipelineStart(CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;
    
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'htsRegs and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((NULL == htsRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS(&htsRegs->PIPELINE_CONTROL[(uint32_t)cfg->pipeline],
                    HTS_PIPELINE_CONTROL_PIPE_EN, 1U);
    }

    return (status);
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion
This portion is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
/* Function to stop the pipeline */
int32_t CSL_htsPipelineStop(CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;

    if ((NULL == htsRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
    {
        CSL_REG32_FINS(&htsRegs->PIPELINE_CONTROL[(uint32_t)cfg->pipeline],
                    HTS_PIPELINE_CONTROL_PIPE_EN, 0U);
    }

    return (status);
}
/* LDRA_JUSTIFY_END */

int32_t CSL_htsCheckThreadConfig(const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to hObj which holds the pointer to cfg, hObj is Fdrv_Handle.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    /* Check for null Pointers */
    if (NULL == cfg)
    {
        status = FVID2_EBADARGS;
    }
    else
/* LDRA_JUSTIFY_END */
    {
        status = HtsCheckConfig(cfg);
    }

    return (status);
}

/* Function to set configuration for complete thread for the given HWA,
   it sets up producer/consumer/DMA scheduler configuration */
int32_t CSL_htsSetThreadConfig(const CSL_htsRegs *htsRegs,
                               const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;
    uint32_t cnt;
    uint32_t htsBaseAddr = (uint32_t )htsRegs;
    const CSL_HtsHwaParams *htsPrms = NULL;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'htsRegs and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    /* Check for null Pointers */
    if ((NULL == htsRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
/* LDRA_JUSTIFY_END */
    {
        status = HtsCheckConfig(cfg);
    }
	
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. Passing only success case to here.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if (FVID2_SOK == status)
/* LDRA_JUSTIFY_END */
    {
        htsPrms = &gCslHtsParams[(uint32_t)cfg->schId];

        /* Set the Scheduler configuration */
        HtsSetSchConfig((htsBaseAddr + htsPrms->schOffset), cfg);

        /* For each of the consumer supported for this HWA,
           Set the consumer and consumer DMA scheduler parameters only if
           enable flag is set to UTRUE,
           Upper layer should set the enable flag to UTRUE for the consumer
           to be used and set others to UFALSE */
        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            if ((uint32_t)UTRUE == cfg->consCfg[cnt].enable)
            {
                /* Set consumer Configuration */
                HtsSetSchConsConfig(
                    (htsBaseAddr + htsPrms->consOffset[cnt]),
                    &cfg->consCfg[cnt]);
            }

            if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enable)
            {
                HtsSetDmaSchProdConfig(
                    (htsBaseAddr + htsPrms->dmaProdOffset[cnt]),
                    &cfg->dmaProdCfg[cnt],
                    htsPrms->isDmaProdPaSupported[cnt]);
            }
        }

        /* For each of the producer supported for this HWA,
           Set the producer and producer DMA scheduler parameters only if
           enable flag is set to UTRUE,
           Upper layer should set the enable flag to UTRUE for the producer
           to be used and set others to UFALSE */
        for (cnt = 0U; cnt < htsPrms->maxProducer; cnt ++)
        {
            if ((uint32_t)UTRUE == cfg->prodCfg[cnt].enable)
            {
                HtsSetSchProdConfig(
                    (htsBaseAddr + htsPrms->prodOffset[cnt]),
                    &cfg->prodCfg[cnt],
                    htsPrms->isPaSupported[cnt]);
            }

            if ((uint32_t)UTRUE == cfg->dmaConsCfg[cnt].enable)
            {
                /* Set consumer DMA scheduler Configuration */
                HtsSetDmaSchConsConfig(
                    (htsBaseAddr + htsPrms->dmaConsOffset[cnt]),
                    &cfg->dmaConsCfg[cnt]);
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion
        This portion is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
        <justification end> */
        if((uint32_t)UTRUE == cfg->dynThldConfig.enable)
        {
            HtsSetDynThldConfig((htsBaseAddr + htsPrms->dynThresholdOffset),
                                 &cfg->dynThldConfig);
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}

int32_t CSL_htsSetThreadUpdateConfig(const CSL_htsRegs *htsRegs,
                                     const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;
    uint32_t cnt;
    uint32_t htsBaseAddr = (uint32_t )htsRegs;
    uint32_t baseAddr;
    volatile uint32_t regVal;
    const CSL_HtsHwaParams *htsPrms = NULL;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'htsRegs and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    /* Check for null Pointers */
    if ((NULL == htsRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }

    if (FVID2_SOK == status)
/* LDRA_JUSTIFY_END */
    {
        htsPrms = &gCslHtsParams[(uint32_t)cfg->schId];

        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enable)
            {
                baseAddr = (htsBaseAddr + htsPrms->dmaProdOffset[cnt]);

                /* Currently only Hop count is updated */
                regVal = CSL_REG32_RD(baseAddr + HWA_DMA_PROD_HOP_OFFSET);
                CSL_FINS(regVal, HTS_DMA0_HOP_HOP_THREAD_COUNT,
                    cfg->dmaProdCfg[cnt].numHop);
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale:
                The value of cfg->dmaProdCfg[cnt].enableHop is always set to UTRUE for enabled input channels during HTS configuration setup, 
                and cannot be modified directly by the application or test framework.
                Effect on this unit:
                No error or adverse effect occurs if the false branch is taken; it disables DMA hop functionality for that channel.
                <justification end> */
                if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enableHop)
                /* LDRA_JUSTIFY_END */
                {
                    CSL_FINS(regVal, HTS_DMA0_HOP_HOP, 1u);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale:
                The value of cfg->dmaProdCfg[cnt].enableHop is always set to UTRUE for enabled input channels during HTS configuration setup, 
                and cannot be modified directly by the application or test framework.
                Effect on this unit:
                No error or adverse effect occurs if the false branch is taken; it disables DMA hop functionality for that channel.
                <justification end> */
                else
                {
                    CSL_FINS(regVal, HTS_DMA0_HOP_HOP, 0u);
                }
                /* LDRA_JUSTIFY_END */
                CSL_REG32_WR((baseAddr + HWA_DMA_PROD_HOP_OFFSET), regVal);
            }
        }
    }

    return (status);
}

/* Function to start the HTS thread and associated consumer/producer sockets */
int32_t CSL_htsThreadStart(const CSL_htsRegs *htsRegs,
                           const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;
    uint32_t cnt;
    uint32_t htsBaseAddr = (uint32_t) htsRegs;
    const CSL_HtsHwaParams *htsPrms = NULL;

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'htsRegs and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((NULL == htsRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
/* LDRA_JUSTIFY_END */
    {
        htsPrms = &gCslHtsParams[cfg->schId];
        /* Start
           1, Consumers
           2, Consumer DMA scheduler
           3, Producer
           4, Producer DMA Scheduler
           5, HWA Scheduler */
        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->dmaProdCfg[cnt].bypass)
            {
                if ((uint32_t)UTRUE == cfg->consCfg[cnt].enable)
                {
                    HtsSchConsStart(
                        (htsBaseAddr + htsPrms->consOffset[cnt]),
                        &cfg->consCfg[cnt]);
                }
                else
                {
                    HtsSchConsStop(
                        (htsBaseAddr + htsPrms->consOffset[cnt]),
                        &cfg->consCfg[cnt]);
                }

                if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enable)
                {
                    HtsDmaSchProdStart(
                        (htsBaseAddr + htsPrms->dmaProdOffset[cnt]),
                        &cfg->dmaProdCfg[cnt],
                        htsPrms->isDmaProdPaSupported[cnt]);
                }
                else
                {
                    HtsDmaSchProdStop(
                        (htsBaseAddr + htsPrms->dmaProdOffset[cnt]),
                        &cfg->dmaProdCfg[cnt],
                        htsPrms->isDmaProdPaSupported[cnt]);
                }
            }
        }

        for (cnt = 0U; cnt < htsPrms->maxProducer; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->dmaConsCfg[cnt].bypass)
            {
                if ((uint32_t)UTRUE == cfg->prodCfg[cnt].enable)
                {
                    HtsSchProdStart(
                        (htsBaseAddr + htsPrms->prodOffset[cnt]),
                        &cfg->prodCfg[cnt],
                        htsPrms->isPaSupported[cnt]);
                }
                else
                {
                    HtsSchProdStop(
                        (htsBaseAddr + htsPrms->prodOffset[cnt]),
                        &cfg->prodCfg[cnt],
                        htsPrms->isPaSupported[cnt]);
                }
                if ((uint32_t)UTRUE == cfg->dmaConsCfg[cnt].enable)
                {
                    HtsDmaSchConsStart(
                        (htsBaseAddr + htsPrms->dmaConsOffset[cnt]),
                        &cfg->dmaConsCfg[cnt]);
                }
                else
                {
                    HtsDmaSchConsStop(
                        (htsBaseAddr + htsPrms->dmaConsOffset[cnt]),
                        &cfg->dmaConsCfg[cnt]);
                }
            }
        }

        /* Set the Scheduler configuration */
        HtsSchStart((htsBaseAddr + htsPrms->schOffset), cfg);
    }

    return (status);
}

/* Function to stop the HTS thread and associated consumer/producer sockets */
void CSL_htsThreadStop(const CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg)
{
    uint32_t cnt;
    uint32_t htsBaseAddr = (uint32_t) htsRegs;
    const CSL_HtsHwaParams *htsPrms = NULL;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'htsRegs and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((NULL != htsRegs) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        htsPrms = &gCslHtsParams[cfg->schId];
        /* Stops
           1, HWA Scheduler
           2, Consumers
           3, Consumer DMA scheduler
           4, Producer
           5, Producer DMA Scheduler */
        HtsSchStop((htsBaseAddr + htsPrms->schOffset), cfg);

        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            if ((uint32_t)UFALSE ==cfg->dmaProdCfg[cnt].bypass)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale:
                The value of cfg->consCfg[cnt].enable is always set to UTRUE for enabled output channels during HTS configuration setup, 
                and cannot be modified directly by the application or test framework.
                Effect on this unit:
                No error or adverse effect occurs if the false branch is taken; it disables consCfg for that ouptut channel.
                <justification end> */
                if ((uint32_t)UTRUE == cfg->consCfg[cnt].enable)
                /* LDRA_JUSTIFY_END */
                {
                    HtsSchConsStop(
                        (htsBaseAddr + htsPrms->consOffset[cnt]),
                        &cfg->consCfg[cnt]);
                }

                if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enable)
                {
                    HtsDmaSchProdStop(
                        (htsBaseAddr + htsPrms->dmaProdOffset[cnt]),
                        &cfg->dmaProdCfg[cnt],
                        htsPrms->isDmaProdPaSupported[cnt]);
                }
            }
        }

        for (cnt = 0U; cnt < htsPrms->maxProducer; cnt ++)
        {
            if ((uint32_t)UFALSE == cfg->dmaConsCfg[cnt].bypass)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale:
                The value of cfg->prodCfg[cnt].enable is always set to UTRUE for enabled output channels during HTS configuration setup, 
                and cannot be modified directly by the application or test framework.
                Effect on this unit:
                No error or adverse effect occurs if the false branch is taken; it disables prodCfg for that ouptut channel.
                <justification end> */
                if ((uint32_t)UTRUE == cfg->prodCfg[cnt].enable)
                /* LDRA_JUSTIFY_END */
                {
                    HtsSchProdStop(
                        (htsBaseAddr + htsPrms->prodOffset[cnt]),
                        &cfg->prodCfg[cnt],
                        htsPrms->isPaSupported[cnt]);
                }

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale:
                The value of cfg->dmaConsCfg[cnt].enable is always set to UTRUE for enabled output channels during HTS configuration setup, 
                and cannot be modified directly by the application or test framework.
                Effect on this unit:
                No error or adverse effect occurs if the false branch is taken; it disables prodCfg for that ouptut channel.
                <justification end> */
                if ((uint32_t)UTRUE == cfg->dmaConsCfg[cnt].enable)
                /* LDRA_JUSTIFY_END */
                {
                    HtsDmaSchConsStop(
                        (htsBaseAddr + htsPrms->dmaConsOffset[cnt]),
                        &cfg->dmaConsCfg[cnt]);
                }
            }
        }
    }
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion
This portion is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
/* Function to stop the HTS thread and associated consumer/producer sockets */
int32_t CSL_htsThreadStopAll(const CSL_htsRegs *htsRegs,
                             const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;
    uint32_t cnt;
    uint32_t htsBaseAddr = (uint32_t) htsRegs;
    const CSL_HtsHwaParams *htsPrms = NULL;

    if ((NULL == htsRegs) || (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
    {
        htsPrms = &gCslHtsParams[cfg->schId];
        /* Stops
           1, HWA Scheduler
           2, Consumers
           3, Consumer DMA scheduler
           4, Producer
           5, Producer DMA Scheduler */
        HtsSchStop((htsBaseAddr + htsPrms->schOffset), cfg);

        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            HtsSchConsStop(
                (htsBaseAddr + htsPrms->consOffset[cnt]),
                &cfg->consCfg[cnt]);

            HtsDmaSchProdStop(
                (htsBaseAddr + htsPrms->dmaProdOffset[cnt]),
                &cfg->dmaProdCfg[cnt], htsPrms->isDmaProdPaSupported[cnt]);
        }

        for (cnt = 0U; cnt < htsPrms->maxProducer; cnt ++)
        {
            HtsSchProdStop(
                (htsBaseAddr + htsPrms->prodOffset[cnt]),
                &cfg->prodCfg[cnt],
                htsPrms->isPaSupported[cnt]);

            HtsDmaSchConsStop(
                (htsBaseAddr + htsPrms->dmaConsOffset[cnt]),
                &cfg->dmaConsCfg[cnt]);
        }
    }

    return (status);
}
/* LDRA_JUSTIFY_END */

uint32_t CSL_htsPipelineStatus(const CSL_htsRegs *htsRegs,
                               const CSL_HtsSchConfig *cfg)
{
    volatile uint32_t pipe_status = 0;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'htsRegs and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((NULL != htsRegs) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        pipe_status = CSL_REG32_FEXT(
            &htsRegs->PIPELINE_CONTROL[(uint32_t)cfg->pipeline],
            HTS_PIPELINE_CONTROL_PIPE_EN);
    }

    return (pipe_status);
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion
This portion is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
int32_t CSL_htsUpdateStreamEn(const CSL_htsRegs *htsRegs,
                              const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;
    uint32_t cnt;
    uint32_t htsBaseAddr = (uint32_t )htsRegs;
    uint32_t addr;
    const CSL_HtsHwaParams *htsPrms = NULL;

    if ((NULL == htsRegs) && (NULL == cfg))
    {
        status = FVID2_EBADARGS;
    }
    else
    {
        htsPrms = &gCslHtsParams[(uint32_t)cfg->schId];

        /* Set the Scheduler STRM_EN bit */
        HtsSetSchConfig((htsBaseAddr + htsPrms->schOffset), cfg);

        addr = htsBaseAddr + htsPrms->schOffset + HWA_SCH_CONTROL_OFFSET;

        if(UTRUE == cfg->enableStream)
        {
            CSL_REG32_FINS(addr, HTS_HWA0_SCHEDULER_CONTROL_STRM_EN, 1U);
        }
        else
        {
            CSL_REG32_FINS(addr, HTS_HWA0_SCHEDULER_CONTROL_STRM_EN, 0U);
        }

        /* Set the DMA Producer STRM_EN bit */
        for (cnt = 0U; cnt < htsPrms->maxConsumer; cnt ++)
        {
            if ((uint32_t)UTRUE == cfg->dmaProdCfg[cnt].enable)
            {
                addr = htsBaseAddr + htsPrms->dmaProdOffset[cnt] +
                            HWA_DMA_PROD_SCH_CONTROL_OFFSET;

                if(UTRUE == cfg->dmaProdCfg[cnt].enableStream)
                {
                    CSL_REG32_FINS(addr, HTS_DMA0_SCHEDULER_CONTROL_STRM_EN, 1U);
                }
                else
                {
                    CSL_REG32_FINS(addr, HTS_DMA0_SCHEDULER_CONTROL_STRM_EN, 0U);
                }

            }
        }

        /* Set the DMA Consumer STRM_EN bit */
        for (cnt = 0U; cnt < htsPrms->maxProducer; cnt ++)
        {
            if ((uint32_t)UTRUE == cfg->dmaConsCfg[cnt].enable)
            {
                addr = htsBaseAddr + htsPrms->dmaConsOffset[cnt] +
                                HWA_DMA_CONS_SCH_CONTROL_OFFSET;

                if(UTRUE == cfg->dmaConsCfg[cnt].enableStream)
                {
                    CSL_REG32_FINS(addr, HTS_DMA240_SCHEDULER_CONTROL_STRM_EN, 1U);
                }
                else
                {
                    CSL_REG32_FINS(addr, HTS_DMA240_SCHEDULER_CONTROL_STRM_EN, 0U);
                }
            }
        }
    }

    return (status);
}
/* LDRA_JUSTIFY_END */

void CSL_htsDisableWatchdogTimer(const CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg)
{
    uint32_t htsBaseAddr;
    const CSL_HtsHwaParams *htsPrms;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((NULL != htsRegs) && (NULL != cfg))
    /* LDRA_JUSTIFY_END */
    {
        htsBaseAddr = (uint32_t)htsRegs;
        htsPrms = &gCslHtsParams[(uint32_t)cfg->schId];
        CSL_REG32_FINS((htsBaseAddr + htsPrms->schOffset + HWA_WDTIMER_OFFSET),
                       HTS_HWA0_WDTIMER_WDTIMER_EN, 0U);
    }
}
/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static void HtsSetSchConfig(uint32_t baseAddr, const CSL_HtsSchConfig *cfg)
{
    uint32_t regVal;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        /* Configure Bandwidth Limiter */
        if ((uint32_t)UTRUE == cfg->enableBwLimit)
        {
            regVal = CSL_REG32_RD(baseAddr + HWA_BW_LIMITER_OFFSET);
            CSL_FINS(regVal,
                HTS_HWA0_BW_LIMITER_BW_LIMITER_EN, 1U);
            CSL_FINS(regVal,
                HTS_HWA0_BW_LIMITER_BW_CYCLE_COUNT, cfg->cycleCnt);
            CSL_FINS(regVal,
                HTS_HWA0_BW_LIMITER_BW_TOKEN_COUNT, cfg->tokenCnt);
            CSL_REG32_WR((baseAddr + HWA_BW_LIMITER_OFFSET), regVal);
        }
        else /* Disable Bandwidth Limiter */
        {
            CSL_REG32_FINS((baseAddr + HWA_BW_LIMITER_OFFSET),
                HTS_HWA0_BW_LIMITER_BW_LIMITER_EN, 0U);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if((uint32_t)UTRUE == cfg->enableTaskSkip)
        {
            regVal = CSL_REG32_RD(baseAddr + HWA_SKIP_CONTROL_OFFSET);
            CSL_FINS(regVal, HTS_HWA0_SKIP_CONTROL_SKIP_BOT, cfg->skipBot);
            CSL_FINS(regVal, HTS_HWA0_SKIP_CONTROL_SKIP_TOP, cfg->skipTop);
            CSL_REG32_WR((baseAddr + HWA_SKIP_CONTROL_OFFSET), regVal);
        }
        /* LDRA_JUSTIFY_END */

        /* Configure Watchdog Timer */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->enableWdTimer)
        /* LDRA_JUSTIFY_END */
        {
            CSL_REG32_FINS((baseAddr + HWA_WDTIMER_OFFSET),
                HTS_HWA0_WDTIMER_WDTIMER_EN, 1U);
            if ((uint32_t)UTRUE == cfg->isWdTimer128KMode)
            {
                CSL_REG32_FINS((baseAddr + HWA_WDTIMER_OFFSET),
                    HTS_HWA0_WDTIMER_WDTIMER_MODE, 1U);
            }
            else
            {
                CSL_REG32_FINS((baseAddr + HWA_WDTIMER_OFFSET),
                    HTS_HWA0_WDTIMER_WDTIMER_MODE, 0U);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            CSL_REG32_FINS((baseAddr + HWA_WDTIMER_OFFSET),
                HTS_HWA0_WDTIMER_WDTIMER_EN, 0U);
        }
        /* LDRA_JUSTIFY_END */

        /* Configure Hop count */
        regVal = CSL_REG32_RD(baseAddr + HWA_HOP_OFFSET);
        CSL_FINS(regVal,
            HTS_HWA0_HOP_HOP_THREAD_COUNT, cfg->numHop);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->enableHop)
        {
            CSL_FINS(regVal, HTS_HWA0_HOP_HOP, 1u);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, HTS_HWA0_HOP_HOP, 0u);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->tdoneCntEnable)
        {
            CSL_FINS(regVal, HTS_HWA0_HOP_TDONE_COUNT_EN, (uint32_t)1U);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, HTS_HWA0_HOP_TDONE_COUNT_EN, (uint32_t)0U);
        }
        CSL_REG32_WR((baseAddr + HWA_HOP_OFFSET), regVal);

        regVal = CSL_REG32_RD(baseAddr + HWA_SCH_CONTROL_OFFSET);
        CSL_FINS(regVal,
            HTS_HWA0_SCHEDULER_CONTROL_PIPELINE_NUM, (uint32_t)cfg->pipeline);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->enableStream)
        {
            CSL_FINS(regVal, HTS_HWA0_SCHEDULER_CONTROL_STRM_EN, 1U);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, HTS_HWA0_SCHEDULER_CONTROL_STRM_EN, 0U);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->isDmaChannel)
        {
            CSL_FINS(regVal, HTS_HWA14_SCHEDULER_CONTROL_DMA_CHANNEL_NO,
                                cfg->dmaChannelNum);
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if((uint32_t)UTRUE == cfg->enableTaskSkip)
        {
            CSL_FINS(regVal, HTS_HWA12_SCHEDULER_CONTROL_SKIP_TASK_EN, (uint32_t)1U);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal, HTS_HWA12_SCHEDULER_CONTROL_SKIP_TASK_EN, (uint32_t)0U);
        }

        CSL_REG32_WR((baseAddr + HWA_SCH_CONTROL_OFFSET), regVal);
    }
}

static void HtsSchStart(uint32_t baseAddr, const CSL_HtsSchConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_SCH_CONTROL_OFFSET),
            HTS_HWA0_SCHEDULER_CONTROL_SCH_EN, 1U);
    }
}

static void HtsSchStop(uint32_t baseAddr, const CSL_HtsSchConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_SCH_CONTROL_OFFSET),
            HTS_HWA0_SCHEDULER_CONTROL_SCH_EN, 0U);
    }
}

static void HtsSetDmaSchConsConfig(uint32_t baseAddr,
								   const CSL_HtsDmaConsConfig *cfg)
{
    uint32_t regVal;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->enable)
        /* LDRA_JUSTIFY_END */
        {
            CSL_REG32_FINS((baseAddr + HWA_DMA_CONS_CONTROL_OFFSET),
                HTS_DMA240_CONS0_CONTROL_PROD_SELECT, (uint32_t)cfg->prodId);

            regVal = CSL_REG32_RD(baseAddr + HWA_DMA_CONS_SCH_CONTROL_OFFSET);
            CSL_FINS(regVal,
                HTS_DMA240_SCHEDULER_CONTROL_DMA_CHANNEL_NO, cfg->dmaChNum);
            CSL_FINS(regVal,
                HTS_DMA240_SCHEDULER_CONTROL_PIPELINE_NUM, (uint32_t)cfg->pipeline);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)UTRUE == cfg->enableStream)
            {
                CSL_FINS(regVal, HTS_DMA240_SCHEDULER_CONTROL_STRM_EN, 1U);
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, HTS_DMA240_SCHEDULER_CONTROL_STRM_EN, 0U);
            }
            CSL_REG32_WR((baseAddr + HWA_DMA_CONS_SCH_CONTROL_OFFSET), regVal);
        }
    }
}

static void HtsDmaSchConsStart(uint32_t baseAddr,
                               const CSL_HtsDmaConsConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_DMA_CONS_CONTROL_OFFSET),
            HTS_DMA240_CONS0_CONTROL_CONS_EN, 1U);

        CSL_REG32_FINS((baseAddr + HWA_DMA_CONS_SCH_CONTROL_OFFSET),
            HTS_DMA240_SCHEDULER_CONTROL_SCH_EN, 1U);
    }
}

static void HtsDmaSchConsStop(uint32_t baseAddr,
                              const CSL_HtsDmaConsConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_DMA_CONS_CONTROL_OFFSET),
            HTS_DMA240_CONS0_CONTROL_CONS_EN, 0U);

        CSL_REG32_FINS((baseAddr + HWA_DMA_CONS_SCH_CONTROL_OFFSET),
            HTS_DMA240_SCHEDULER_CONTROL_SCH_EN, 0U);
    }
}

static void HtsSetDmaSchProdConfig(uint32_t baseAddr,
                        const CSL_HtsDmaProdConfig *cfg, uint32_t isPaSupported)
{
    uint32_t regVal;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((uint32_t)UTRUE == cfg->enable)
        /* LDRA_JUSTIFY_END */
        {
            regVal = CSL_REG32_RD(baseAddr + HWA_DMA_PROD_HOP_OFFSET);
            CSL_FINS(regVal, HTS_DMA0_HOP_HOP_THREAD_COUNT, cfg->numHop);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)UTRUE == cfg->enableHop)
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, HTS_DMA0_HOP_HOP, 1u);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                CSL_FINS(regVal, HTS_DMA0_HOP_HOP, 0u);
            }
            /* LDRA_JUSTIFY_END */
            CSL_REG32_WR((baseAddr + HWA_DMA_PROD_HOP_OFFSET), regVal);

            CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_CONTROL_OFFSET),
                HTS_DMA0_PROD0_CONTROL_CONS_SELECT, (uint32_t)cfg->consId);

            regVal = CSL_REG32_RD(baseAddr + HWA_DMA_PROD_BUF_CONTROL_OFFSET);
            CSL_FINS(regVal, HTS_DMA0_PROD0_BUF_CONTROL_COUNT_DEC,
                cfg->countDec);
            CSL_FINS(regVal, HTS_DMA0_PROD0_BUF_CONTROL_THRESHOLD,
                cfg->threshold);
            CSL_FINS(regVal, HTS_DMA0_PROD0_BUF_CONTROL_DEPTH, cfg->depth);
            CSL_REG32_WR((baseAddr + HWA_DMA_PROD_BUF_CONTROL_OFFSET), regVal);

            regVal = CSL_REG32_RD(baseAddr + HWA_DMA_PROD_COUNT_OFFSET);
            CSL_FINS(regVal, HTS_DMA0_PROD0_COUNT_COUNT_POSTLOAD,
                cfg->cntPostLoad);
            CSL_FINS(regVal, HTS_DMA0_PROD0_COUNT_COUNT_PRELOAD,
                cfg->cntPreLoad);
            CSL_REG32_WR((baseAddr + HWA_DMA_PROD_COUNT_OFFSET), regVal);

            if (((uint32_t)UTRUE == isPaSupported) &&
                (UTRUE == cfg->paCfg.enable))
            {
                CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_PA_PROD_COUNT_OFFSET),
                    HTS_DMA8_PA0_PRODCOUNT_PA_COUNT_DEC, cfg->paCfg.countDec);

                regVal = CSL_REG32_RD(baseAddr + HWA_DMA_PROD_PA_CONTROL_OFFSET);
                CSL_FINS(regVal, HTS_DMA8_PA0_CONTROL_PA_PS_MAXCOUNT,
                    cfg->paCfg.psMaxCnt);
                CSL_FINS(regVal, HTS_DMA8_PA0_CONTROL_PA_CS_MAXCOUNT,
                    cfg->paCfg.csMaxCnt);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0u != cfg->paCfg.enableBufCtrl)
                {
                    CSL_FINS(regVal, HTS_DMA8_PA0_CONTROL_PA_BUF_CNTL, 1U);
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    CSL_FINS(regVal, HTS_DMA8_PA0_CONTROL_PA_BUF_CNTL, 0U);
                }

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0u != cfg->paCfg.enableDecCtrl)
                /* LDRA_JUSTIFY_END */
                {
                    CSL_FINS(regVal, HTS_DMA8_PA0_CONTROL_PA_DEC_CNTL, 1U);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    CSL_FINS(regVal, HTS_DMA8_PA0_CONTROL_PA_DEC_CNTL, 0U);
                }
                /* LDRA_JUSTIFY_END */

                CSL_REG32_WR((baseAddr + HWA_DMA_PROD_PA_CONTROL_OFFSET),
                    regVal);
            }

            regVal = CSL_REG32_RD(baseAddr + HWA_DMA_PROD_SCH_CONTROL_OFFSET);
            CSL_FINS(regVal, HTS_DMA0_SCHEDULER_CONTROL_DMA_CHANNEL_NO,
                cfg->dmaChNum);
            CSL_FINS(regVal, HTS_DMA0_SCHEDULER_CONTROL_PIPELINE_NUM,
                (uint32_t)cfg->pipeline);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((uint32_t)UTRUE == cfg->enableStream)
            {
                CSL_FINS(regVal, HTS_DMA0_SCHEDULER_CONTROL_STRM_EN, 1U);
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, HTS_DMA0_SCHEDULER_CONTROL_STRM_EN, 0U);
            }
            CSL_REG32_WR((baseAddr + HWA_DMA_PROD_SCH_CONTROL_OFFSET), regVal);

            if (((uint32_t)UTRUE == isPaSupported) &&
                (UTRUE == cfg->paCfg.enable))
            {
                CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_PA_PROD_COUNT_OFFSET),
                    HTS_DMA9_PA0_PRODCOUNT_PA_COUNT_DEC, cfg->paCfg.countDec);

                regVal = CSL_REG32_RD(baseAddr +
                    HWA_DMA_PROD_PA_CONTROL_OFFSET);
                CSL_FINS(regVal, HTS_DMA9_PA0_CONTROL_PA_PS_MAXCOUNT,
                    cfg->paCfg.psMaxCnt);
                CSL_FINS(regVal, HTS_DMA9_PA0_CONTROL_PA_CS_MAXCOUNT,
                    cfg->paCfg.csMaxCnt);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0u != cfg->paCfg.enableBufCtrl)
                {
                    CSL_FINS(regVal, HTS_DMA9_PA0_CONTROL_PA_BUF_CNTL, 1U);
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    CSL_FINS(regVal, HTS_DMA9_PA0_CONTROL_PA_BUF_CNTL, 0U);
                }

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                if (0u != cfg->paCfg.enableDecCtrl)
                /* LDRA_JUSTIFY_END */
                {
                    CSL_FINS(regVal, HTS_DMA9_PA0_CONTROL_PA_DEC_CNTL, 1U);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    CSL_FINS(regVal, HTS_DMA9_PA0_CONTROL_PA_DEC_CNTL, 0U);
                }
                /* LDRA_JUSTIFY_END */

                CSL_REG32_WR((baseAddr + HWA_DMA_PROD_PA_CONTROL_OFFSET),
                    regVal);
            }
            else
            {
                CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_PA_CONTROL_OFFSET),
                    HTS_DMA9_PA0_CONTROL_PA_ENABLE, 0U);
            }
        }
    }
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale:
The test framework and test apps cannot reach this portion
This portion is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit:
None; Unused feature, cannot be enabled, control cannot reach here.
<justification end> */
static void HtsSetDynThldConfig(uint32_t baseAddr,
                                const CSL_HtsDynThldConfig *cfg)
{
    uint32_t cnt;
    uint32_t regVal;

    if ((0U != baseAddr) && (NULL != cfg))
    {
        for(cnt = 0; cnt < cfg->numEntries; cnt++)
        {
            regVal = 0;
            CSL_FINS(regVal, HTS_HWA26_DYNAMIC_THRESHOLD0_DYN_COUNT_DEC,
                    cfg->dynamicConfig[cnt].countDec);
            CSL_FINS(regVal, HTS_HWA26_DYNAMIC_THRESHOLD0_DYN_CS_MAXCOUNT,
                    cfg->dynamicConfig[cnt].csMaxCnt);
            CSL_REG32_WR((baseAddr + (cnt * HWA_DYN_THRESHOLD_OFFSET)),
                    regVal);
        }
    }
}
/* LDRA_JUSTIFY_END */

static void HtsDmaSchProdStart(uint32_t baseAddr,
    const CSL_HtsDmaProdConfig *cfg, uint32_t isPaSupported)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_CONTROL_OFFSET),
            HTS_DMA0_PROD0_CONTROL_PROD_EN, 1U);
        CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_SCH_CONTROL_OFFSET),
            HTS_DMA0_SCHEDULER_CONTROL_SCH_EN, 1U);

        if (((uint32_t)UTRUE == isPaSupported) && (UTRUE == cfg->paCfg.enable))
        {
            CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_PA_CONTROL_OFFSET),
                HTS_DMA8_PA0_CONTROL_PA_ENABLE, 1U);
        }
    }
}

static void HtsDmaSchProdStop(uint32_t baseAddr,
    const CSL_HtsDmaProdConfig *cfg, uint32_t isPaSupported)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_CONTROL_OFFSET),
            HTS_DMA0_PROD0_CONTROL_PROD_EN, 0U);
        CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_SCH_CONTROL_OFFSET),
            HTS_DMA0_SCHEDULER_CONTROL_SCH_EN, 0U);

        if (((uint32_t)UTRUE == isPaSupported) && (UTRUE == cfg->paCfg.enable))
        {
            CSL_REG32_FINS((baseAddr + HWA_DMA_PROD_PA_CONTROL_OFFSET),
                HTS_DMA8_PA0_CONTROL_PA_ENABLE, 0U);
        }
    }
}

static void HtsSetSchConsConfig(uint32_t baseAddr, const CSL_HtsConsConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_CONS_CONTROL_OFFSET),
            HTS_HWA0_CONS0_CONTROL_PROD_SELECT, (uint32_t)cfg->prodId);
    }
}

static void HtsSchConsStart(uint32_t baseAddr, const CSL_HtsConsConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_CONS_CONTROL_OFFSET),
            HTS_HWA0_CONS0_CONTROL_CONS_EN, 1U);
    }
}

static void HtsSchConsStop(uint32_t baseAddr, const CSL_HtsConsConfig *cfg)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_CONS_CONTROL_OFFSET),
            HTS_HWA0_CONS0_CONTROL_CONS_EN, 0U);
    }
}

static void HtsSetSchProdConfig(uint32_t baseAddr, const CSL_HtsProdConfig *cfg,
    uint32_t isPaSupported)
{
    uint32_t regVal;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        regVal = CSL_REG32_RD(baseAddr + HWA_PROD_CONTROL_OFFSET);
        CSL_FINS(regVal,
                HTS_HWA0_PROD0_CONTROL_CONS_SELECT, (uint32_t)cfg->consId);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UTRUE == cfg->isMaskSelect)
        {
            CSL_FINS(regVal,
                    HTS_HWA0_PROD0_CONTROL_MASK_SELECT, cfg->maskSelect);
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UTRUE == cfg->skipEnable)
        {
            CSL_FINS(regVal,
                    HTS_HWA0_PROD0_CONTROL_SKIP_PEND_EN, (uint32_t)1U);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal,
                    HTS_HWA0_PROD0_CONTROL_SKIP_PEND_EN, (uint32_t)0U);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if(UTRUE == cfg->dynthldEnable)
        {
            CSL_FINS(regVal,
                    HTS_HWA26_PROD0_CONTROL_DYNAMIC_TH_EN, (uint32_t)1U);
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            CSL_FINS(regVal,
                    HTS_HWA26_PROD0_CONTROL_DYNAMIC_TH_EN, (uint32_t)0U);
        }
        CSL_REG32_WR((baseAddr + HWA_PROD_CONTROL_OFFSET), regVal);

        regVal = CSL_REG32_RD(baseAddr + HWA_PROD_BUF_CONTROL_OFFSET);
        CSL_FINS(regVal, HTS_HWA0_PROD0_BUF_CONTROL_COUNT_DEC, cfg->countDec);
        CSL_FINS(regVal, HTS_HWA0_PROD0_BUF_CONTROL_THRESHOLD, cfg->threshold);
        CSL_FINS(regVal, HTS_HWA0_PROD0_BUF_CONTROL_DEPTH, cfg->depth);
        CSL_REG32_WR((baseAddr + HWA_PROD_BUF_CONTROL_OFFSET), regVal);

        regVal = CSL_REG32_RD(baseAddr + HWA_PROD_COUNT_OFFSET);
        CSL_FINS(regVal, HTS_HWA0_PROD0_COUNT_COUNT_POSTLOAD, cfg->cntPostLoad);
        CSL_FINS(regVal, HTS_HWA0_PROD0_COUNT_COUNT_PRELOAD, cfg->cntPreLoad);
        CSL_REG32_WR((baseAddr + HWA_PROD_COUNT_OFFSET), regVal);

        if (((uint32_t)UTRUE == isPaSupported) && (UTRUE == cfg->paCfg.enable))
        {
            CSL_REG32_FINS((baseAddr + HWA_PROD_PA_PROD_COUNT_OFFSET),
                HTS_HWA0_PA0_PRODCOUNT_PA_COUNT_DEC, cfg->paCfg.countDec);

            regVal = CSL_REG32_RD(baseAddr + HWA_PROD_PA_CONTROL_OFFSET);
            CSL_FINS(regVal, HTS_HWA0_PA0_CONTROL_PA_PS_MAXCOUNT,
                cfg->paCfg.psMaxCnt);
            CSL_FINS(regVal, HTS_HWA0_PA0_CONTROL_PA_CS_MAXCOUNT,
                cfg->paCfg.csMaxCnt);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0u != cfg->paCfg.enableBufCtrl)
            {
                CSL_FINS(regVal, HTS_HWA0_PA0_CONTROL_PA_BUF_CNTL, 1U);
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, HTS_HWA0_PA0_CONTROL_PA_BUF_CNTL, 0U);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0u != cfg->paCfg.enableDecCtrl)
            /* LDRA_JUSTIFY_END */
            {
                CSL_FINS(regVal, HTS_HWA0_PA0_CONTROL_PA_DEC_CNTL, 1U);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                CSL_FINS(regVal, HTS_HWA0_PA0_CONTROL_PA_DEC_CNTL, 0U);
            }
            /* LDRA_JUSTIFY_END */

            CSL_REG32_WR((baseAddr + HWA_PROD_PA_CONTROL_OFFSET), regVal);
        }
    }
}

static void HtsSchProdStart(uint32_t baseAddr, const CSL_HtsProdConfig *cfg,
    uint32_t isPaSupported)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_PROD_CONTROL_OFFSET),
            HTS_HWA0_PROD0_CONTROL_PROD_EN, 1U);

        if (((uint32_t)UTRUE == isPaSupported) && (UTRUE == cfg->paCfg.enable))
        {
            CSL_REG32_FINS((baseAddr + HWA_PROD_PA_CONTROL_OFFSET),
                HTS_HWA0_PA0_CONTROL_PA_ENABLE, 1U);
        }
        else
        {
            CSL_REG32_FINS((baseAddr + HWA_PROD_PA_CONTROL_OFFSET),
                HTS_HWA0_PA0_CONTROL_PA_ENABLE, 0U);
        }
    }
}

static void HtsSchProdStop(uint32_t baseAddr, const CSL_HtsProdConfig *cfg,
    uint32_t isPaSupported)
{
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> 
Rationale: The negative test framework and test apps cannot reach the false case of this condition. 
           'baseAddr and cfg' are passed using Fvid2Control from Imaging kernel layer along with pointer to instObj which holds the pointer to htsRegs, instObj is defined as a global gM2mVissInstObj.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
    if ((0U != baseAddr) && (NULL != cfg))
/* LDRA_JUSTIFY_END */
    {
        CSL_REG32_FINS((baseAddr + HWA_PROD_CONTROL_OFFSET),
            HTS_HWA0_PROD0_CONTROL_PROD_EN, 0U);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((uint32_t)UTRUE == isPaSupported) && (UTRUE == cfg->paCfg.enable))
        {
            CSL_REG32_FINS((baseAddr + HWA_PROD_PA_CONTROL_OFFSET),
                HTS_HWA0_PA0_CONTROL_PA_ENABLE, 0U);
        }
        /* LDRA_JUSTIFY_END */
    }
}

static int32_t HtsCheckConfig(const CSL_HtsSchConfig *cfg)
{
    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
    <justification end> */
    if(UTRUE == cfg->dynThldConfig.enable)
    {
        if((CSL_HTS_HWA_SCH_SP14 != cfg->schId) &&
           (CSL_HTS_HWA_SCH_SP15 != cfg->schId))
       {
            status = FVID2_EINVALID_PARAMS;
       }
       if(cfg->dynThldConfig.numEntries > CSL_HTS_MAX_DYN_THRSHOLD)
       {
           status = FVID2_EINVALID_PARAMS;
       }
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

const CSL_HtsHwaParams *GetVpacHtsParams(uint32_t schId)
{
    return &gCslHtsParams[schId];
}