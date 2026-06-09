/*
 * Copyright (c) 2025 Texas Instruments Incorporated
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
 * \file sciserver_secproxySocData.h
 *
 * \brief SoC defines for secure proxy instances for J722S device
 *
 */
#ifndef SOC_J722S_SPROXY_SOC_DATA_H
#define SOC_J722S_SPROXY_SOC_DATA_H


/** Number of Secure Proxy instances */
#define SOC_MAX_SEC_PROXY_INSTANCES (0x02U)
/*
 * J722S_SPROXY_IDX_DMASS0_SEC_PROXY_0: Secure proxy instance:
 * DMASS0_SEC_PROXY_0 linked to RA inst: DMASS0_RINGACC_0
 */
#define J722S_SPROXY_IDX_DMASS0_SEC_PROXY_0 (0x00U)
/*
 * J722S_SPROXY_IDX_SA3_SS0_SEC_PROXY_0: Secure proxy instance:
 * SA3_SS0_SEC_PROXY_0 linked to RA inst: SA3_SS0_RINGACC_0
 */
#define J722S_SPROXY_IDX_SA3_SS0_SEC_PROXY_0 (0x01U)

/** Secure Proxy instance used by ROM */
#define ROM_SEC_PROXY_IDX J722S_SPROXY_IDX_SA3_SS0_SEC_PROXY_0

#endif /* SOC_J722S_SPROXY_SOC_DATA_H */
