/*
 * UDMAP
 *
 * UDMAP driver instance data definition
 *
 * Data version: 220525_125254
 *
 * Copyright (C) 2021-2025, Texas Instruments Incorporated
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
#include <soc/j721s2/resasg_types.h>
#include <soc/j721s2/soc_devgrps.h>

#include "../../include/udmap_inst.h"
#include "udmap_cfg.h"

static const struct rm_mmr mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x311a0000U,
};

static const struct rm_mmr mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_TCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x35840000U,
};

static const struct rm_mmr mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_TCHANRT = {
	.fwl_id		= 4388U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 15U,
	.base		= 0x35c00000U,
};

static const struct rm_mmr mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_RCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x35880000U,
};

static const struct rm_mmr mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_RCHANRT = {
	.fwl_id		= 4388U,
	.fwl_ch_start	= 16U,
	.fwl_ch_end	= 47U,
	.base		= 0x35d00000U,
};

static const struct udmap_ch_type rx_ch_types_NAVSS0_BCDMA_0[1] = {
	{
		.start = 0U,
		.end = 31U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_RX_HCCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_BCDMA_0, RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
	},
};

static const struct udmap_ch_type tx_ch_types_NAVSS0_BCDMA_0[1] = {
	{
		.start = 0U,
		.end = 15U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = 0U,
		.err_oes_offset = 0U,
		.type = DMSS_BCDMA_TX_HCCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_BCDMA_0, RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
	},
};

static const struct rm_mmr mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x31150000U,
};

static const struct rm_mmr mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_TCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x30b00000U,
};

static const struct rm_mmr mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_TCHANRT = {
	.fwl_id		= 4368U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 340U,
	.base		= 0x35000000U,
};

static const struct rm_mmr mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x30c00000U,
};

static const struct rm_mmr mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RCHANRT = {
	.fwl_id		= 4368U,
	.fwl_ch_start	= 341U,
	.fwl_ch_end	= 422U,
	.base		= 0x34000000U,
};

static const struct rm_mmr mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RFLOW = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x30d00000U,
};

static const struct udmap_ch_type rx_ch_types_NAVSS0_UDMAP_0[3] = {
	{
		.start = 0U,
		.end = 1U,
		.flow_start = 0U,
		.flow_end = 1U,
		.flow_utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_UHCHAN),
		.oes_offset = UDMAP_RCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_RCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_RX_UHCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_UHCHAN),
	},
	{
		.start = 2U,
		.end = 3U,
		.flow_start = 2U,
		.flow_end = 3U,
		.flow_utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_HCHAN),
		.oes_offset = UDMAP_RCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_RCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_RX_HCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_HCHAN),
	},
	{
		.start = 4U,
		.end = 81U,
		.flow_start = 4U,
		.flow_end = 81U,
		.flow_utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_CHAN),
		.oes_offset = UDMAP_RCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_RCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_RX_CHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_CHAN),
	},
};

static const struct udmap_ch_type tx_ch_types_NAVSS0_UDMAP_0[4] = {
	{
		.start = 0U,
		.end = 1U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = UDMAP_TCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_TCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_TX_UHCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_TX_UHCHAN),
	},
	{
		.start = 2U,
		.end = 3U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = UDMAP_TCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_TCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_TX_HCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_TX_HCHAN),
	},
	{
		.start = 4U,
		.end = 84U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = UDMAP_TCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_TCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_TX_CHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_TX_CHAN),
	},
	{
		.start = 85U,
		.end = 340U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = UDMAP_TCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_TCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_TX_ECHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_TX_ECHAN),
	},
};

static u8 flow_delegates_NAVSS0_UDMAP_0[142U];

static struct udmap_flow_common common_flows_NAVSS0_UDMAP_0 = {
	.start		= UDMAP_NAVSS0_UDMAP_0_RX_CHAN_MAX_INDEX,
	.end		= UDMAP_NAVSS0_UDMAP_0_RX_FLOW_MAX_INDEX - 1U,
	.utype		= RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0,     RESASG_SUBTYPE_UDMAP_RX_FLOW_COMMON),
	.delegates	= &flow_delegates_NAVSS0_UDMAP_0[0U],
};

static const struct rm_mmr mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x285c0000U,
};

static const struct rm_mmr mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_TCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x284a0000U,
};

static const struct rm_mmr mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP_TCHANRT = {
	.fwl_id		= 6244U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 47U,
	.base		= 0x2aa00000U,
};

static const struct rm_mmr mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_RCHAN = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x284c0000U,
};

static const struct rm_mmr mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP_RCHANRT = {
	.fwl_id		= 6244U,
	.fwl_ch_start	= 48U,
	.fwl_ch_end	= 95U,
	.base		= 0x2a800000U,
};

static const struct rm_mmr mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RFLOW = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x28400000U,
};

static const struct udmap_ch_type rx_ch_types_MCU_NAVSS0_UDMAP_0[2] = {
	{
		.start = 0U,
		.end = 1U,
		.flow_start = 0U,
		.flow_end = 1U,
		.flow_utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_HCHAN),
		.oes_offset = UDMAP_RCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_RCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_RX_HCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_HCHAN),
	},
	{
		.start = 2U,
		.end = 47U,
		.flow_start = 2U,
		.flow_end = 47U,
		.flow_utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_CHAN),
		.oes_offset = UDMAP_RCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_RCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_RX_CHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_RX_CHAN),
	},
};

static const struct udmap_ch_type tx_ch_types_MCU_NAVSS0_UDMAP_0[2] = {
	{
		.start = 0U,
		.end = 1U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = UDMAP_TCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_TCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_TX_HCHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_TX_HCHAN),
	},
	{
		.start = 2U,
		.end = 47U,
		.flow_start = 0U,
		.flow_end = 0U,
		.flow_utype = 0U,
		.oes_offset = UDMAP_TCHAN_OES_EVT_INDEX_OFFSET,
		.err_oes_offset = UDMAP_TCHAN_EOES_EVT_INDEX_OFFSET,
		.type = UDMAP_TX_CHAN,
		.utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_TX_CHAN),
	},
};

static u8 flow_delegates_MCU_NAVSS0_UDMAP_0[48U];

static struct udmap_flow_common common_flows_MCU_NAVSS0_UDMAP_0 = {
	.start		= UDMAP_MCU_NAVSS0_UDMAP_0_RX_CHAN_MAX_INDEX,
	.end		= UDMAP_MCU_NAVSS0_UDMAP_0_RX_FLOW_MAX_INDEX - 1U,
	.utype		= RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0,	  RESASG_SUBTYPE_UDMAP_RX_FLOW_COMMON),
	.delegates	= &flow_delegates_MCU_NAVSS0_UDMAP_0[0U],
};

/*
 * Resources used by ROM which must have their OES and EOES registers reset
 */
static struct udmap_used_mapping rom_usage_MCU_NAVSS0_UDMAP_0[4U] = {
	{
		.ch = 0U,
		.tx_ch = STRUE,
		.err_evt = SFALSE,
		.cleared = SFALSE,
	},
	{
		.ch = 0U,
		.tx_ch = STRUE,
		.err_evt = STRUE,
		.cleared = SFALSE,
	},
	{
		.ch = 0U,
		.tx_ch = SFALSE,
		.err_evt = SFALSE,
		.cleared = SFALSE,
	},
	{
		.ch = 0U,
		.tx_ch = SFALSE,
		.err_evt = STRUE,
		.cleared = SFALSE,
	},
};

struct udmap_instance udmap_inst[3] = {
	{
		.id = J721S2_DEV_NAVSS0_BCDMA_0,
		.root_id = J721S2_DEV_NAVSS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_J721S2_MAIN,
		.gcfg = &mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_GCFG,
		.tchan = &mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_TCHAN,
		.rchan = &mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_RCHAN,
		.bchan = NULL,
		.rflow = NULL,
		.tchanrt = &mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_TCHANRT,
		.rchanrt = &mmr_NAVSS0_BCDMA_0_BCDMA0_CFG_RCHANRT,
		.bchanrt = NULL,
		.n_tx_ch = UDMAP_NAVSS0_BCDMA_0_TX_CHAN_MAX_INDEX,
		.n_rx_ch = UDMAP_NAVSS0_BCDMA_0_RX_CHAN_MAX_INDEX,
		.n_bc_ch = 0U,
		.n_rx_flow = 0U,
		.bc_ch_offset = 0U,
		.tx_ch_types = &tx_ch_types_NAVSS0_BCDMA_0[0U],
		.rx_ch_types = &rx_ch_types_NAVSS0_BCDMA_0[0U],
		.bc_ch_types = NULL,
		.common_flow = NULL,
		.n_tx_ch_type = ARRAY_SIZE(tx_ch_types_NAVSS0_BCDMA_0),
		.n_rx_ch_type = ARRAY_SIZE(rx_ch_types_NAVSS0_BCDMA_0),
		.n_bc_ch_type = 0U,
		.invalid_flow_oes_offset = 0U,
		.invalid_flow_oes_utype = 0U,
		.gcfg_type = BCDMA_GCFG,
		.gcfg_utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_BCDMA_0, RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
		.rom_usage = NULL,
		.n_rom_usage = 0U,
		.dru_ch0_dst_thread_offset = 0x0U,
	},
	{
		.id = J721S2_DEV_NAVSS0_UDMAP_0,
		.root_id = J721S2_DEV_NAVSS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_J721S2_MAIN,
		.gcfg = &mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG,
		.tchan = &mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_TCHAN,
		.rchan = &mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RCHAN,
		.bchan = NULL,
		.rflow = &mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RFLOW,
		.tchanrt = &mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_TCHANRT,
		.rchanrt = &mmr_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RCHANRT,
		.bchanrt = NULL,
		.n_tx_ch = UDMAP_NAVSS0_UDMAP_0_TX_CHAN_MAX_INDEX,
		.n_rx_ch = UDMAP_NAVSS0_UDMAP_0_RX_CHAN_MAX_INDEX,
		.n_bc_ch = 0U,
		.n_rx_flow = UDMAP_NAVSS0_UDMAP_0_RX_FLOW_MAX_INDEX,
		.bc_ch_offset = 0U,
		.tx_ch_types = &tx_ch_types_NAVSS0_UDMAP_0[0U],
		.rx_ch_types = &rx_ch_types_NAVSS0_UDMAP_0[0U],
		.bc_ch_types = NULL,
		.common_flow = &common_flows_NAVSS0_UDMAP_0,
		.n_tx_ch_type = ARRAY_SIZE(tx_ch_types_NAVSS0_UDMAP_0),
		.n_rx_ch_type = ARRAY_SIZE(rx_ch_types_NAVSS0_UDMAP_0),
		.n_bc_ch_type = 0U,
		.invalid_flow_oes_offset = UDMAP_GCFG_INVALID_FLOW_EVT_INDEX,
		.invalid_flow_oes_utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_INVALID_FLOW_OES),
		.gcfg_type = UDMAP_GCFG,
		.gcfg_utype = RESASG_UTYPE(J721S2_DEV_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
		.rom_usage = NULL,
		.n_rom_usage = 0U,
		.dru_ch0_dst_thread_offset = 0xc800U,
	},
	{
		.id = J721S2_DEV_MCU_NAVSS0_UDMAP_0,
		.root_id = J721S2_DEV_MCU_NAVSS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_J721S2_MCU_WAKEUP,
		.gcfg = &mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_GCFG,
		.tchan = &mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_TCHAN,
		.rchan = &mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_RCHAN,
		.bchan = NULL,
		.rflow = &mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP0_CFG_RFLOW,
		.tchanrt = &mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP_TCHANRT,
		.rchanrt = &mmr_MCU_NAVSS0_UDMAP_0_UDMASS_UDMAP_RCHANRT,
		.bchanrt = NULL,
		.n_tx_ch = UDMAP_MCU_NAVSS0_UDMAP_0_TX_CHAN_MAX_INDEX,
		.n_rx_ch = UDMAP_MCU_NAVSS0_UDMAP_0_RX_CHAN_MAX_INDEX,
		.n_bc_ch = 0U,
		.n_rx_flow = UDMAP_MCU_NAVSS0_UDMAP_0_RX_FLOW_MAX_INDEX,
		.bc_ch_offset = 0U,
		.tx_ch_types = &tx_ch_types_MCU_NAVSS0_UDMAP_0[0U],
		.rx_ch_types = &rx_ch_types_MCU_NAVSS0_UDMAP_0[0U],
		.bc_ch_types = NULL,
		.common_flow = &common_flows_MCU_NAVSS0_UDMAP_0,
		.n_tx_ch_type = ARRAY_SIZE(tx_ch_types_MCU_NAVSS0_UDMAP_0),
		.n_rx_ch_type = ARRAY_SIZE(rx_ch_types_MCU_NAVSS0_UDMAP_0),
		.n_bc_ch_type = 0U,
		.invalid_flow_oes_offset = UDMAP_GCFG_INVALID_FLOW_EVT_INDEX,
		.invalid_flow_oes_utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_INVALID_FLOW_OES),
		.gcfg_type = UDMAP_GCFG,
		.gcfg_utype = RESASG_UTYPE(J721S2_DEV_MCU_NAVSS0_UDMAP_0, RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
		.rom_usage = &rom_usage_MCU_NAVSS0_UDMAP_0[0U],
		.n_rom_usage = ARRAY_SIZE(rom_usage_MCU_NAVSS0_UDMAP_0),
		.dru_ch0_dst_thread_offset = 0xc800U,
	},
};

const u32 udmap_inst_count = ARRAY_SIZE(udmap_inst);
