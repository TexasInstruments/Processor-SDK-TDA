/*
 * UDMAP
 *
 * UDMAP driver instance data definition
 *
 * Data version: 250904_170808
 *
 * Copyright (C) 2023-2026, Texas Instruments Incorporated
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
#include <soc/am62px/resasg_types.h>
#include <soc/am62px/soc_devgrps.h>

#include "../../include/udmap_inst.h"
#include "udmap_cfg.h"

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x485c0100U,
};

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_TCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x484a4000U,
};

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_TCHANRT = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4aa40000U,
};

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_RCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x484c2000U,
};

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_RCHANRT = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4a820000U,
};

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_BCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x48420000U,
};

static const struct rm_mmr mmr_DMASS0_BCDMA_0_BCDMA_BCHANRT = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4c000000U,
};

static const struct udmap_ch_type bc_ch_types_DMASS0_BCDMA_0[2] = {
	{
		.start = 0U,
		.end = 3U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_BLOCK_COPY_HCCHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_BCDMA_0, RESASG_SUBTYPE_BCDMA_BLOCK_COPY_HCCHAN),
	},
	{
		.start = 4U,
		.end = 31U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_BLOCK_COPY_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_BCDMA_0, RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
	},
};

static const struct udmap_ch_type rx_ch_types_DMASS0_BCDMA_0[1] = {
	{
		.start = 0U,
		.end = 24U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_BCDMA_0, RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
	},
};

static const struct udmap_ch_type tx_ch_types_DMASS0_BCDMA_0[1] = {
	{
		.start = 0U,
		.end = 24U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_TX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_BCDMA_0, RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
	},
};

static const struct rm_mmr mmr_DMASS0_PKTDMA_0_PKTDMA_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x485c0000U,
};

static const struct rm_mmr mmr_DMASS0_PKTDMA_0_PKTDMA_TCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x484a0000U,
};

static const struct rm_mmr mmr_DMASS0_PKTDMA_0_PKTDMA_TCHANRT = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4aa00000U,
};

static const struct rm_mmr mmr_DMASS0_PKTDMA_0_PKTDMA_RCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x484c0000U,
};

static const struct rm_mmr mmr_DMASS0_PKTDMA_0_PKTDMA_RCHANRT = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4a800000U,
};

static const struct rm_mmr mmr_DMASS0_PKTDMA_0_PKTDMA_RFLOW = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x48430000U,
};

static const struct udmap_ch_type rx_ch_types_DMASS0_PKTDMA_0[6] = {
	{
		.start = 0U,
		.end = 18U,
		.flow_start = 0U,
		.flow_end = 18U,
		.flow_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
	},
	{
		.start = 19U,
		.end = 19U,
		.flow_start = 19U,
		.flow_end = 34U,
		.flow_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_FLOW_CPSW_RX_CHAN),
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_CPSW_RX_CHAN),
	},
	{
		.start = 20U,
		.end = 20U,
		.flow_start = 35U,
		.flow_end = 42U,
		.flow_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_0_CHAN),
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_SAUL_RX_0_CHAN),
	},
	{
		.start = 21U,
		.end = 21U,
		.flow_start = 35U,
		.flow_end = 42U,
		.flow_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_1_CHAN),
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_SAUL_RX_1_CHAN),
	},
	{
		.start = 22U,
		.end = 22U,
		.flow_start = 43U,
		.flow_end = 50U,
		.flow_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_2_CHAN),
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_SAUL_RX_2_CHAN),
	},
	{
		.start = 23U,
		.end = 23U,
		.flow_start = 43U,
		.flow_end = 50U,
		.flow_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_3_CHAN),
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_SAUL_RX_3_CHAN),
	},
};

static const struct udmap_ch_type tx_ch_types_DMASS0_PKTDMA_0[4] = {
	{
		.start = 0U,
		.end = 18U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_TX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
	},
	{
		.start = 19U,
		.end = 26U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_TX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_CPSW_TX_CHAN),
	},
	{
		.start = 27U,
		.end = 27U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_TX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_SAUL_TX_0_CHAN),
	},
	{
		.start = 28U,
		.end = 28U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_PKTDMA_TX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_PKTDMA_SAUL_TX_1_CHAN),
	},
};

/*
 * Resources used by ROM which must have their OES and EOES registers reset
 */
static struct udmap_used_mapping rom_usage_DMASS0_PKTDMA_0[4U] = {
	{
		.ch = 19U,
		.tx_ch = STRUE,
		.err_evt = SFALSE,
		.cleared = SFALSE,
	},
	{
		.ch = 19U,
		.tx_ch = STRUE,
		.err_evt = STRUE,
		.cleared = SFALSE,
	},
	{
		.ch = 19U,
		.tx_ch = SFALSE,
		.err_evt = SFALSE,
		.cleared = SFALSE,
	},
	{
		.ch = 19U,
		.tx_ch = SFALSE,
		.err_evt = STRUE,
		.cleared = SFALSE,
	},
};

static const struct rm_mmr mmr_DMASS1_BCDMA_0_BCDMA_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4e230000U,
};

static const struct rm_mmr mmr_DMASS1_BCDMA_0_BCDMA_RCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4e200000U,
};

static const struct rm_mmr mmr_DMASS1_BCDMA_0_BCDMA_RCHANRT = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x4e180000U,
};

static const struct udmap_ch_type rx_ch_types_DMASS1_BCDMA_0[1] = {
	{
		.start = 0U,
		.end = 5U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_RX_CHAN,
		.utype = RESASG_UTYPE(AM62PX_DEV_DMASS1_BCDMA_0, RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
	},
};

struct udmap_instance udmap_inst[3] = {
	{
		.id = AM62PX_DEV_DMASS0_BCDMA_0,
		.root_id = AM62PX_DEV_DMASS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM62PX_MAIN,
		.gcfg = &mmr_DMASS0_BCDMA_0_BCDMA_GCFG,
		.tchan = &mmr_DMASS0_BCDMA_0_BCDMA_TCHAN,
		.rchan = &mmr_DMASS0_BCDMA_0_BCDMA_RCHAN,
		.bchan = &mmr_DMASS0_BCDMA_0_BCDMA_BCHAN,
		.rflow = NULL,
		.tchanrt = &mmr_DMASS0_BCDMA_0_BCDMA_TCHANRT,
		.rchanrt = &mmr_DMASS0_BCDMA_0_BCDMA_RCHANRT,
		.bchanrt = &mmr_DMASS0_BCDMA_0_BCDMA_BCHANRT,
		.n_tx_ch = UDMAP_DMASS0_BCDMA_0_TX_CHAN_MAX_INDEX,
		.n_rx_ch = UDMAP_DMASS0_BCDMA_0_RX_CHAN_MAX_INDEX,
		.n_bc_ch = UDMAP_DMASS0_BCDMA_0_BC_CHAN_MAX_INDEX,
		.n_rx_flow = 0U,
		.bc_ch_offset = UDMAP_DMASS0_BCDMA_0_BC_CHAN_OFFSET,
		.tx_ch_types = &tx_ch_types_DMASS0_BCDMA_0[0U],
		.rx_ch_types = &rx_ch_types_DMASS0_BCDMA_0[0U],
		.bc_ch_types = &bc_ch_types_DMASS0_BCDMA_0[0U],
		.common_flow = NULL,
		.n_tx_ch_type = ARRAY_SIZE(tx_ch_types_DMASS0_BCDMA_0),
		.n_rx_ch_type = ARRAY_SIZE(rx_ch_types_DMASS0_BCDMA_0),
		.n_bc_ch_type = ARRAY_SIZE(bc_ch_types_DMASS0_BCDMA_0),
		.invalid_flow_oes_offset = 0U,
		.invalid_flow_oes_utype = 0U,
		.gcfg_type = BCDMA_GCFG,
		.gcfg_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_BCDMA_0, RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
		.rom_usage = NULL,
		.n_rom_usage = 0U,
		.dru_ch0_dst_thread_offset = 0x0U,
	},
	{
		.id = AM62PX_DEV_DMASS0_PKTDMA_0,
		.root_id = AM62PX_DEV_DMASS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM62PX_MAIN,
		.gcfg = &mmr_DMASS0_PKTDMA_0_PKTDMA_GCFG,
		.tchan = &mmr_DMASS0_PKTDMA_0_PKTDMA_TCHAN,
		.rchan = &mmr_DMASS0_PKTDMA_0_PKTDMA_RCHAN,
		.bchan = NULL,
		.rflow = &mmr_DMASS0_PKTDMA_0_PKTDMA_RFLOW,
		.tchanrt = &mmr_DMASS0_PKTDMA_0_PKTDMA_TCHANRT,
		.rchanrt = &mmr_DMASS0_PKTDMA_0_PKTDMA_RCHANRT,
		.bchanrt = NULL,
		.n_tx_ch = UDMAP_DMASS0_PKTDMA_0_TX_CHAN_MAX_INDEX,
		.n_rx_ch = UDMAP_DMASS0_PKTDMA_0_RX_CHAN_MAX_INDEX,
		.n_bc_ch = 0U,
		.n_rx_flow = UDMAP_DMASS0_PKTDMA_0_RX_FLOW_MAX_INDEX,
		.bc_ch_offset = 0U,
		.tx_ch_types = &tx_ch_types_DMASS0_PKTDMA_0[0U],
		.rx_ch_types = &rx_ch_types_DMASS0_PKTDMA_0[0U],
		.bc_ch_types = NULL,
		.common_flow = NULL,
		.n_tx_ch_type = ARRAY_SIZE(tx_ch_types_DMASS0_PKTDMA_0),
		.n_rx_ch_type = ARRAY_SIZE(rx_ch_types_DMASS0_PKTDMA_0),
		.n_bc_ch_type = 0U,
		.invalid_flow_oes_offset = 0U,
		.invalid_flow_oes_utype = 0U,
		.gcfg_type = PKTDMA_GCFG,
		.gcfg_utype = RESASG_UTYPE(AM62PX_DEV_DMASS0_PKTDMA_0, RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
		.rom_usage = &rom_usage_DMASS0_PKTDMA_0[0U],
		.n_rom_usage = ARRAY_SIZE(rom_usage_DMASS0_PKTDMA_0),
		.dru_ch0_dst_thread_offset = 0x0U,
	},
	{
		.id = AM62PX_DEV_DMASS1_BCDMA_0,
		.root_id = AM62PX_DEV_DMASS1,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM62PX_MAIN,
		.gcfg = &mmr_DMASS1_BCDMA_0_BCDMA_GCFG,
		.tchan = NULL,
		.rchan = &mmr_DMASS1_BCDMA_0_BCDMA_RCHAN,
		.bchan = NULL,
		.rflow = NULL,
		.tchanrt = NULL,
		.rchanrt = &mmr_DMASS1_BCDMA_0_BCDMA_RCHANRT,
		.bchanrt = NULL,
		.n_tx_ch = UDMAP_DMASS1_BCDMA_0_TX_CHAN_MAX_INDEX,
		.n_rx_ch = UDMAP_DMASS1_BCDMA_0_RX_CHAN_MAX_INDEX,
		.n_bc_ch = 0U,
		.n_rx_flow = 0U,
		.bc_ch_offset = 0U,
		.tx_ch_types = NULL,
		.rx_ch_types = &rx_ch_types_DMASS1_BCDMA_0[0U],
		.bc_ch_types = NULL,
		.common_flow = NULL,
		.n_tx_ch_type = 0U,
		.n_rx_ch_type = ARRAY_SIZE(rx_ch_types_DMASS1_BCDMA_0),
		.n_bc_ch_type = 0U,
		.invalid_flow_oes_offset = 0U,
		.invalid_flow_oes_utype = 0U,
		.gcfg_type = BCDMA_GCFG,
		.gcfg_utype = RESASG_UTYPE(AM62PX_DEV_DMASS1_BCDMA_0, RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
		.rom_usage = NULL,
		.n_rom_usage = 0U,
		.dru_ch0_dst_thread_offset = 0x0U,
	},
};

const u32 udmap_inst_count = ARRAY_SIZE(udmap_inst);
