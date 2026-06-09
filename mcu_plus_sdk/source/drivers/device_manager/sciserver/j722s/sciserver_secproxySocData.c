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
 * \file sciserver_secproxySocData.c
 *
 * \brief SoC Data for Secure Proxy Instances for J722S device
 *
 */

#include <types/errno.h>
#include <types/short_types.h>
#include <sciserver_commMemorySoc.h>
#include <soc_devgrps.h>

#include <sciserver_secproxyDescription.h>
#include <sciserver_secproxySocData.h>
#include <sciserver_ringAcelSocData.h>

const uint8_t Sciserver_SecureProxyInstancesNum = SOC_MAX_SEC_PROXY_INSTANCES;

const struct Sciserver_SecureProxy gSciserverSecureProxyInstances[SOC_MAX_SEC_PROXY_INSTANCES] = {
	/*
	 * [0] J722S_SPROXY_IDX_DMASS0_SEC_PROXY_0: Secure proxy instance:
	 * DMASS0_SEC_PROXY_0 linked to RA inst: DMASS0_RINGACC_0
	 */
	[J722S_SPROXY_IDX_DMASS0_SEC_PROXY_0] =	 {
		.baseaddr_cfg_rt		= 0x4A600000ULL,
		.baseaddr_cfg_scfg		= 0x4A400000ULL,
		.baseaddr_src_target_data	= 0x4D000000ULL,
		.sproxy_target_hl		= 0x00400000ULL,
		.sproxy_ring_index_base		= 0U,
		.sproxy_ring_index_max		= 19U,
		.num_threads			= 76U,
		.devgrp				= SOC_DEVGRP_J722S_TIFS_INTERNAL,
		.ra_idx				= J722S_RA_IDX_DMASS0_RINGACC_0,
		.buffer_idx			= J722S_SPROXY_MEM_IDX_DMSS_IPC_SRAM,
		.buffer_neg_offset		= 0x00001300U,
	},
	/*
	 * [1] J722S_SPROXY_IDX_SA3_SS0_SEC_PROXY_0: Secure proxy instance:
	 * SA3_SS0_SEC_PROXY_0 linked to RA inst: SA3_SS0_RINGACC_0
	 */
	[J722S_SPROXY_IDX_SA3_SS0_SEC_PROXY_0] = {
		.baseaddr_cfg_rt		= 0x44880000ULL,
		.baseaddr_cfg_scfg		= 0x44860000ULL,
		.baseaddr_src_target_data	= 0x43600000ULL,
		.sproxy_target_hl		= 0x00400000ULL,
		.sproxy_ring_index_base		= 0U,
		.sproxy_ring_index_max		= 5U,
		.num_threads			= 16U,
		.devgrp				= SOC_DEVGRP_J722S_HSM_INTERNAL,
		.ra_idx				= J722S_RA_IDX_SA3_SS0_RINGACC_0,
		.buffer_idx			= J722S_SPROXY_MEM_IDX_DMSS_HSM_IPC_SRAM,
		.buffer_neg_offset		= 0x00000400U,
	},
};
