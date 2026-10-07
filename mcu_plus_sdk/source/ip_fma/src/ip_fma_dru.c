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
 *  \ingroup  IP_FMA_DRU
 *  \defgroup IP_FMA_CSIRX_IMPLEMENTATION DRU module Register Check Implementation
 *
 *  @{
 */

/**
 *  \file     ip_fma_dru.c
 *
 *  \brief    DRU-specific wrappers over IP_FMA Register Check.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "ip_fma_dru.h"
#include <drivers/hw_include/dru/v2/csl_dru.h>
#include <drivers/hw_include/dru/v2/cslr_dru.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static IpFma_RegDesc gIpFma_DruQueueRegDescArr[] =
{
    { CSL_DRU_QUEUE_CFG(0x0U),    0u,     IPFMA_WIDTH_64 },
    { CSL_DRU_QUEUE_CFG(0x1U),    0u,     IPFMA_WIDTH_64 },
    { CSL_DRU_QUEUE_CFG(0x2U),    0u,     IPFMA_WIDTH_64 },
    { CSL_DRU_QUEUE_CFG(0x3U),    0u,     IPFMA_WIDTH_64 },
    { CSL_DRU_QUEUE_CFG(0x4U),    0u,     IPFMA_WIDTH_64 },
};

static IpFma_RegDesc gIpFma_DruChnrtRegDescArr[] =
{
    { CSL_DRU_CHNRT_CHST_SCHED(0x0U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x4U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x5U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x6U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x7U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x8U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x9U),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0xAU),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0xBU),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0xCU),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0xDU),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0xEU),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0xFU),  0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x10U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x11U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x12U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x13U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x14U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x15U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x16U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x17U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x18U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x19U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1AU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1BU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1CU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1DU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1EU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x1FU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x20U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x21U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x22U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x23U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x24U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x25U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x26U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x27U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x28U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x29U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2AU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2BU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2CU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2DU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2EU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x2FU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x30U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x31U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x32U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x33U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x34U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x35U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x36U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x37U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x38U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x39U), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3AU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3BU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3CU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3DU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3EU), 0u, IPFMA_WIDTH_64 },
    { CSL_DRU_CHNRT_CHST_SCHED(0x3FU), 0u, IPFMA_WIDTH_64 },
};

/* ========================================================================== */
/*                          Function Definitions                               */
/* ========================================================================== */

IpFma_Status IpFma_Dru_GetQueueRegs(uintptr_t baseAddr, IpFma_DruQueueRegs* out)
{
    IpFma_Status status = IPFMA_E_PARAM;
    uint32_t i;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        status = IpFma_GetRegsValues(baseAddr, gIpFma_DruQueueRegDescArr,
                                        (uint32_t)(sizeof(gIpFma_DruQueueRegDescArr) / sizeof(gIpFma_DruQueueRegDescArr[0])));

        if (IPFMA_OK == status)
        {
            for (i = 0; i < DRU_NUM_QUEUES; i++)
            {
                out->cfg[i]  = gIpFma_DruQueueRegDescArr[i].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Dru_CompareQueueRegs(const IpFma_DruQueueRegs* expected,
                                              const IpFma_DruQueueRegs* actual)
{
     IpFma_Status status = IPFMA_OK;
     uint32_t i;

    if (((NULL_PTR == expected)) || ((NULL_PTR == actual)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        for (i = 0; i < DRU_NUM_QUEUES; i++)
        {
            if ((expected->cfg[i] != actual->cfg[i]))
            {
                status = IPFMA_E_MISMATCH;
                break;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Dru_GetChnrtRegs(uintptr_t baseAddr, IpFma_DruChnrtRegs* out)
{
    IpFma_Status status = IPFMA_E_PARAM;
    uint32_t i;

    if ((NULL_PTR == out))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        status = IpFma_GetRegsValues(baseAddr, gIpFma_DruChnrtRegDescArr,
                                        (uint32_t)(sizeof(gIpFma_DruChnrtRegDescArr) / sizeof(gIpFma_DruChnrtRegDescArr[0])));

        if (IPFMA_OK == status)
        {
            for (i = 0; i < DRU_NUM_CHANNELS; i++)
            {
                out->chst_sched[i] = gIpFma_DruChnrtRegDescArr[i].value;
            }
        }
    }

    return status;
}

IpFma_Status IpFma_Dru_CompareChnrtRegs(const IpFma_DruChnrtRegs* expected,
                                              const IpFma_DruChnrtRegs* actual)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t i;

    if (((NULL_PTR == expected)) || ((NULL_PTR == actual)))
    {
        status = IPFMA_E_PARAM;
    }
    else
    {
        for (i = 0; i < DRU_NUM_CHANNELS; i++)
        {
            if ((expected->chst_sched[i] != actual->chst_sched[i]))
            {
                status = IPFMA_E_MISMATCH;
                break;
            }
        }
    }

    return status;
}

/** @} */
