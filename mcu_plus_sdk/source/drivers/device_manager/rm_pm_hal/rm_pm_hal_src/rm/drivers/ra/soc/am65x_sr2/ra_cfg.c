/*
 * Ring Accelerator
 *
 * Ring accelerator driver instance data definition
 *
 * Data version: 000000_000000
 *
 * Copyright (C) 2019-2026 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <types/short_types.h>
#include <types/array_size.h>
#include <types/sbool.h>
#include <soc/rm_mmr.h>
#include <devices.h>
#include <soc/am65x_sr2/resasg_types.h>
#include <soc/am65x_sr2/soc_devgrps.h>

#include "../../include/ra_inst.h"
#include "ra_cfg.h"

static const struct rm_mmr mmr_navss0_ringacc0_UDMASS_RINGACC0_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x31160000U,
};

static const struct rm_mmr mmr_navss0_ringacc0_UDMASS_RINGACC0_SRC_FIFOS = {
	.fwl_id		= 4416U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 817U,
	.base		= 0x38000000U,
};

static const struct rm_mmr mmr_navss0_ringacc0_UDMASS_RINGACC0_CFG_MON = {
	.fwl_id		= 4256U,
	.fwl_ch_start	= 818U,
	.fwl_ch_end	= 849U,
	.base		= 0x32000000U,
};

static const struct rm_mmr mmr_navss0_ringacc0_UDMASS_RINGACC0_CFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x31080000U,
};

static const struct rm_mmr mmr_navss0_ringacc0_UDMASS_RINGACC0_CFG_RT = {
	.fwl_id		= 4256U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 817U,
	.base		= 0x3c000000U,
};

static const struct ra_ring_type ring_types_navss0_ringacc0[6] = {
	{
		.start = 0U,
		.end = 7U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_TX_H),
	},
	{
		.start = 8U,
		.end = 119U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_TX),
	},
	{
		.start = 120U,
		.end = 151U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_TX_EXT),
	},
	{
		.start = 152U,
		.end = 159U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_RX_H),
	},
	{
		.start = 160U,
		.end = 301U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_RX),
	},
	{
		.start = 302U,
		.end = 767U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_GP),
	},
};

static const struct ra_ring_type ring_mon_types_navss0_ringacc0[1] = {
	{
		.start = 0U,
		.end = 31U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_MONITORS),
	},
};

/*
 * Resources used by ROM which must have their OES registers reset
 */
static struct ra_used_mapping rom_usage_navss0_ringacc0[1U] = {
	{
		.ring = 304U,
		.cleared = SFALSE,
	},
};

static const struct rm_mmr mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG_GCFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x285d0000U,
};

static const struct rm_mmr mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_FIFOS = {
	.fwl_id		= 6272U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 285U,
	.base		= 0x2b000000U,
};

static const struct rm_mmr mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG_MON = {
	.fwl_id		= 6160U,
	.fwl_ch_start	= 286U,
	.fwl_ch_end	= 317U,
	.base		= 0x2a280000U,
};

static const struct rm_mmr mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG = {
	.fwl_id		= RM_MMR_FWL_INVALID,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 0U,
	.base		= 0x28440000U,
};

static const struct rm_mmr mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG_RT = {
	.fwl_id		= 6160U,
	.fwl_ch_start	= 0U,
	.fwl_ch_end	= 285U,
	.base		= 0x2b800000U,
};

static const struct ra_ring_type ring_types_mcu_navss0_ringacc0[5] = {
	{
		.start = 0U,
		.end = 1U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_TX_H),
	},
	{
		.start = 2U,
		.end = 47U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_TX),
	},
	{
		.start = 48U,
		.end = 49U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_RX_H),
	},
	{
		.start = 50U,
		.end = 95U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_UDMAP_RX),
	},
	{
		.start = 96U,
		.end = 255U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_GP),
	},
};

static const struct ra_ring_type ring_mon_types_mcu_navss0_ringacc0[1] = {
	{
		.start = 0U,
		.end = 31U,
		.type = RA_STANDARD_RING,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_MONITORS),
	},
};

/*
 * Resources used by ROM which must have their OES registers reset
 */
static struct ra_used_mapping rom_usage_mcu_navss0_ringacc0[8U] = {
	{
		.ring = 0U,
		.cleared = SFALSE,
	},
	{
		.ring = 96U,
		.cleared = SFALSE,
	},
	{
		.ring = 97U,
		.cleared = SFALSE,
	},
	{
		.ring = 98U,
		.cleared = SFALSE,
	},
	{
		.ring = 256U,
		.cleared = SFALSE,
	},
	{
		.ring = 257U,
		.cleared = SFALSE,
	},
	{
		.ring = 258U,
		.cleared = SFALSE,
	},
	{
		.ring = 259U,
		.cleared = SFALSE,
	},
};

struct ra_instance ra_inst[2] = {
	{
		.id = AM6_DEV_NAVSS0_RINGACC0,
		.root_id = AM6_DEV_NAVSS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM6_MAIN,
		.gcfg = &mmr_navss0_ringacc0_UDMASS_RINGACC0_GCFG,
		.cfg = &mmr_navss0_ringacc0_UDMASS_RINGACC0_CFG,
		.rt = &mmr_navss0_ringacc0_UDMASS_RINGACC0_CFG_RT,
		.mon = &mmr_navss0_ringacc0_UDMASS_RINGACC0_CFG_MON,
		.fifos = &mmr_navss0_ringacc0_UDMASS_RINGACC0_SRC_FIFOS,
		.n_gp_rings_pre_sp_rings = RA_NAVSS0_RINGACC0_MAX_INDEX_PRE_SP,
		.n_gp_rings_post_sp_rings = RA_NAVSS0_RINGACC0_MAX_INDEX_POST_SP,
		.gp_ring_oes_offset_pre_sp = RA_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_PRE_SP,
		.gp_ring_oes_offset_post_sp = RA_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_POST_SP,
		.ring_types = &ring_types_navss0_ringacc0[0U],
		.n_ring_type = ARRAY_SIZE(ring_types_navss0_ringacc0),
		.ring_mon_types = &ring_mon_types_navss0_ringacc0[0U],
		.n_ring_mon_type = ARRAY_SIZE(ring_mon_types_navss0_ringacc0),
		.err_evt_index = RA_GCFG_ERROR_EVT_INDEX,
		.err_evt_utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_ERROR_OES),
		.virtid_utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_VIRTID),
		.n_ring_mon = RA_NAVSS0_RINGACC0_MONITOR_MAX_INDEX,
		.ring_mon_utype = RESASG_UTYPE(AM6_DEV_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_MONITORS),
		.ring_mon_oes_offset = RA_MON_OES_EVT_INDEX_OFFSET,
		.rom_usage = &rom_usage_navss0_ringacc0[0U],
		.n_rom_usage = ARRAY_SIZE(rom_usage_navss0_ringacc0),
	},
	{
		.id = AM6_DEV_MCU_NAVSS0_RINGACC0,
		.root_id = AM6_DEV_MCU_NAVSS0,
		.initialized = SFALSE,
		.devgrp = SOC_DEVGRP_AM6_MCU_WAKEUP | SOC_DEVGRP_AM6_DMSC_INTERNAL,
		.gcfg = &mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG_GCFG,
		.cfg = &mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG,
		.rt = &mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG_RT,
		.mon = &mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_CFG_MON,
		.fifos = &mmr_mcu_navss0_ringacc0_UDMASS_RINGACC0_FIFOS,
		.n_gp_rings_pre_sp_rings = RA_MCU_NAVSS0_RINGACC0_MAX_INDEX_PRE_SP,
		.n_gp_rings_post_sp_rings = RA_MCU_NAVSS0_RINGACC0_MAX_INDEX_POST_SP,
		.gp_ring_oes_offset_pre_sp = RA_MCU_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_PRE_SP,
		.gp_ring_oes_offset_post_sp = RA_MCU_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_POST_SP,
		.ring_types = &ring_types_mcu_navss0_ringacc0[0U],
		.n_ring_type = ARRAY_SIZE(ring_types_mcu_navss0_ringacc0),
		.ring_mon_types = &ring_mon_types_mcu_navss0_ringacc0[0U],
		.n_ring_mon_type = ARRAY_SIZE(ring_mon_types_mcu_navss0_ringacc0),
		.err_evt_index = RA_GCFG_ERROR_EVT_INDEX,
		.err_evt_utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_ERROR_OES),
		.virtid_utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_VIRTID),
		.n_ring_mon = RA_MCU_NAVSS0_RINGACC0_MONITOR_MAX_INDEX,
		.ring_mon_utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_RINGACC0, RESASG_SUBTYPE_RA_MONITORS),
		.ring_mon_oes_offset = RA_MON_OES_EVT_INDEX_OFFSET,
		.rom_usage = &rom_usage_mcu_navss0_ringacc0[0U],
		.n_rom_usage = ARRAY_SIZE(rom_usage_mcu_navss0_ringacc0),
	},
};

const u32 ra_inst_count = ARRAY_SIZE(ra_inst);
