/*
 * Proxy
 *
 * Proxy driver instance data definition
 *
 * Data version: 000000_000000
 *
 * Copyright (C) 2026, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
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

#include <types/short_types.h>
#include <types/array_size.h>
#include <types/sbool.h>
#include <soc/rm_mmr.h>
#include <devices.h>
#include <soc/am65x_sr2/resasg_types.h>
#include <soc/am65x_sr2/soc_devgrps.h>

#include "../../include/proxy_inst.h"
#include "proxy_cfg.h"

static const struct rm_mmr mmr_navss0_proxy0_PROXY0_BUF_CFG = {
	.fwl_id		= 4148U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 63U,
	.base		= 0x33400000U,
};

static const struct rm_mmr mmr_navss0_proxy0_PROXY_TARGET0_DATA = {
	.fwl_id		= 4392U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 63U,
	.base		= 0x33000000U,
};

static const struct rm_mmr mmr_mcu_navss0_proxy0_PROXY0_BUF_CFG = {
	.fwl_id		= 6146U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 63U,
	.base		= 0x2a580000U,
};

static const struct rm_mmr mmr_mcu_navss0_proxy0_PROXY0_TARGET0_DATA = {
	.fwl_id		= 6256U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 63U,
	.base		= 0x2a500000U,
};

struct proxy_instance proxy_inst[PROXY_INST_COUNT] = {
	{
		.id = AM6_DEV_NAVSS0_PROXY0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM6_MAIN,
		.cfg = &mmr_navss0_proxy0_PROXY0_BUF_CFG,
		.target0_data = &mmr_navss0_proxy0_PROXY_TARGET0_DATA,
		.n_proxies = PROXY_NAVSS0_PROXY0_PROXY_MAX_INDEX,
		.proxy_utype = RESASG_UTYPE(AM6_DEV_NAVSS0_PROXY0, RESASG_SUBTYPE_PROXY_PROXIES),
	},
	{
		.id = AM6_DEV_MCU_NAVSS0_PROXY0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM6_MCU_WAKEUP,
		.cfg = &mmr_mcu_navss0_proxy0_PROXY0_BUF_CFG,
		.target0_data = &mmr_mcu_navss0_proxy0_PROXY0_TARGET0_DATA,
		.n_proxies = PROXY_MCU_NAVSS0_PROXY0_PROXY_MAX_INDEX,
		.proxy_utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_PROXY0, RESASG_SUBTYPE_PROXY_PROXIES),
	},
};
