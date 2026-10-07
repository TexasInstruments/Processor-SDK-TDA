/*
 * Copyright (c) 2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 *  \file sciclient_merged.c
 *
 *  \brief File containing the Sciclient driver APIs.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/csl/csl_types.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/sciclient/src/sec_proxy/csl_sec_proxy.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_priv.h>

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint32_t gSecContextId;
extern uint32_t gNonSecContextId;
extern CSL_SecProxyCfg gSciclient_secProxyCfg;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Sciclient_init(const Sciclient_ConfigPrms_t *pCfgPrms)
{
    int32_t   status = CSL_PASS;

    status = Sciclient_mergedInit();

    if (status == CSL_PASS)
    {
        status = Sciclient_initInternal(pCfgPrms);
    }

    return status;
}

int32_t Sciclient_mergedInit(void)
{
    int32_t ret = CSL_PASS;
    uint32_t coreId = SCICLIENT_CORE_INVALID;

    coreId = Sciclient_getR5CoreId();
    switch (coreId)
    {
    case SCICLIENT_CORE_MCU1_0:
        gSecContextId = SCICLIENT_CONTEXT_R5_SEC_0;
        gNonSecContextId = SCICLIENT_CONTEXT_R5_NONSEC_0;
        break;
    case SCICLIENT_CORE_MCU1_1:
        gSecContextId = SCICLIENT_CONTEXT_R5_SEC_1;
        gNonSecContextId = SCICLIENT_CONTEXT_R5_NONSEC_1;
        break;
    case SCICLIENT_CORE_MCU2_0:
        gSecContextId = SCICLIENT_CONTEXT_MAIN_0_R5_SEC_0;
        gNonSecContextId = SCICLIENT_CONTEXT_MAIN_0_R5_NONSEC_0;
        break;
    case SCICLIENT_CORE_MCU2_1:
        gSecContextId = SCICLIENT_CONTEXT_MAIN_0_R5_SEC_1;
        gNonSecContextId = SCICLIENT_CONTEXT_MAIN_0_R5_NONSEC_1;
        break;
#if defined(SOC_J721E) || defined(SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    case SCICLIENT_CORE_MCU3_0:
        gSecContextId = SCICLIENT_CONTEXT_MAIN_1_R5_SEC_0;
        gNonSecContextId = SCICLIENT_CONTEXT_MAIN_1_R5_NONSEC_0;
        break;
    case SCICLIENT_CORE_MCU3_1:
        gSecContextId = SCICLIENT_CONTEXT_MAIN_1_R5_SEC_1;
        gNonSecContextId = SCICLIENT_CONTEXT_MAIN_1_R5_NONSEC_1;
        break;
#if defined (SOC_J784S4) || defined (SOC_J742S2)
    case SCICLIENT_CORE_MCU4_0:
        gSecContextId = SCICLIENT_CONTEXT_MAIN_2_R5_SEC_0;
        gNonSecContextId = SCICLIENT_CONTEXT_MAIN_2_R5_NONSEC_0;
        break;
    case SCICLIENT_CORE_MCU4_1:
        gSecContextId = SCICLIENT_CONTEXT_MAIN_2_R5_SEC_1;
        gNonSecContextId = SCICLIENT_CONTEXT_MAIN_2_R5_NONSEC_1;
        break;
#endif
#endif
    default:
        ret = CSL_EFAIL;
        break;
    }

    if (ret == CSL_PASS)
    {
        if ((SCICLIENT_CORE_MCU1_1 < coreId) && (SCICLIENT_CORE_INVALID != coreId))
        {
            gSciclient_secProxyCfg.pSecProxyRegs = (CSL_sec_proxyRegs *)CSL_NAVSS0_SEC_PROXY0_CFG_MMRS_BASE;
            gSciclient_secProxyCfg.pSecProxyScfgRegs = (CSL_sec_proxy_scfgRegs *)CSL_NAVSS0_SEC_PROXY0_CFG_SCFG_BASE;
            gSciclient_secProxyCfg.pSecProxyRtRegs = (CSL_sec_proxy_rtRegs *)CSL_NAVSS0_SEC_PROXY0_CFG_RT_BASE;
            gSciclient_secProxyCfg.proxyTargetAddr = (uint64_t)CSL_NAVSS0_SEC_PROXY0_SRC_TARGET_DATA_BASE;
            gSciclient_secProxyCfg.maxMsgSize = 0;
        }
    }

    return ret;
}

int32_t Sciclient_service(const Sciclient_ReqPrm_t *pReqPrm,
                          Sciclient_RespPrm_t      *pRespPrm)
{
    int32_t ret = CSL_EFAIL;
    uint32_t coreId = Sciclient_getR5CoreId();

    switch (coreId)
    {
    case SCICLIENT_CORE_MCU1_0:
        ret = Sciclient_serviceDirect(pReqPrm, pRespPrm);
        break;
    case SCICLIENT_CORE_MCU1_1:
    case SCICLIENT_CORE_MCU2_0:
    case SCICLIENT_CORE_MCU2_1:
#if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    case SCICLIENT_CORE_MCU3_0:
    case SCICLIENT_CORE_MCU3_1:
#if defined (SOC_J784S4) || defined (SOC_J742S2)
    case SCICLIENT_CORE_MCU4_0:
    case SCICLIENT_CORE_MCU4_1:
#endif /* #if defined (SOC_J784S4) || defined (SOC_J742S2) */
#endif /* #if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2) */
        ret = Sciclient_serviceIndirect(pReqPrm, pRespPrm);
        break;
    default:
        break;
    }

    return ret;
}

uint32_t Sciclient_getCurrentContext(const Sciclient_ReqPrm_t *pReqPrm)
{
    uint32_t ret = SCICLIENT_CONTEXT_MAX_NUM;
    uint32_t coreId = Sciclient_getR5CoreId();

    switch (coreId)
    {
    case SCICLIENT_CORE_MCU1_0:
        ret = Sciclient_getCurrentContextDirect(pReqPrm);
        break;
    case SCICLIENT_CORE_MCU1_1:
    case SCICLIENT_CORE_MCU2_0:
    case SCICLIENT_CORE_MCU2_1:
#if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    case SCICLIENT_CORE_MCU3_0:
    case SCICLIENT_CORE_MCU3_1:
#if defined (SOC_J784S4) || defined (SOC_J742S2)
    case SCICLIENT_CORE_MCU4_0:
    case SCICLIENT_CORE_MCU4_1:
#endif /* #if defined (SOC_J784S4) || defined (SOC_J742S2) */
#endif /* #if defined (SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2) */
        ret = Sciclient_getCurrentContextIndirect(pReqPrm);;
        break;
    default:
        break;
    }

    return ret;
}