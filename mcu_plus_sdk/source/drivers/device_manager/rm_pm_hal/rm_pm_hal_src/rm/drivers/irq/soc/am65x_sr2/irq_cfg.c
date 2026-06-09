/*
 * Interrupt Management
 *
 * Interrupt management driver instance data definition
 *
 * Data version: 000000_000000
 *
 * Copyright (C) 2019-2026 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <types/short_types.h>
#include <types/array_size.h>
#include <devices.h>
#include <devices_internal.h>
#include <hosts.h>
#include <hosts_internal.h>
#include <soc/am65x_sr2/resasg_types.h>

#include "../../include/irq_resasg.h"
#include <irq_cfg.h>

/*
 * SoC devices capable of sourcing global events
 */
const u16 evt_rt_srcs[10] = {
	AM6_DEV_NAVSS0_PROXY0,
	AM6_DEV_NAVSS0_RINGACC0,
	AM6_DEV_NAVSS0_TIMER_MGR0,
	AM6_DEV_NAVSS0_TIMER_MGR1,
	AM6_DEV_NAVSS0_UDMAP0,
	AM6_DEV_NAVSS0_UDMASS_INTA0,
	AM6_DEV_MCU_NAVSS0_PROXY0,
	AM6_DEV_MCU_NAVSS0_RINGACC0,
	AM6_DEV_MCU_NAVSS0_UDMAP0,
	AM6_DEV_MCU_NAVSS0_INTR_AGGR_0,
};

const u32 evt_rt_srcs_count = ARRAY_SIZE(evt_rt_srcs);

const struct irq_global_event_type irq_global_event_types[IRQ_GLOBAL_EVENT_TYPES_ID_MAX] = {
	{
		.start = 0U,
		.num = 4608U,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_UDMASS_INTA0, RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
	},
	{
		.start = 16384U,
		.num = 1536U,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_INTR_AGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
	},
	{
		.start = 20480U,
		.num = 1024U,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_MODSS_INTA0, RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
	},
	{
		.start = 22528U,
		.num = 1024U,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_MODSS_INTA1, RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
	},
	{
		.start = 32768U,
		.num = 512U,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_UDMASS_INTA0, RESASG_SUBTYPE_GLOBAL_EVENT_MEVT),
	},
	{
		.start = 34816U,
		.num = 128U,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_INTR_AGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_MEVT),
	},
	{
		.start = 36864U,
		.num = 512U,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_UDMASS_INTA0, RESASG_SUBTYPE_GLOBAL_EVENT_GEVT),
	},
	{
		.start = 39936U,
		.num = 256U,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_INTR_AGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_GEVT),
	},
	{
		.start = 49152U,
		.num = 1024U,
		.utype = RESASG_UTYPE(AM6_DEV_NAVSS0_UDMAP0, RESASG_SUBTYPE_GLOBAL_EVENT_TRIGGER),
	},
	{
		.start = 56320U,
		.num = 256U,
		.utype = RESASG_UTYPE(AM6_DEV_MCU_NAVSS0_UDMAP0, RESASG_SUBTYPE_GLOBAL_EVENT_TRIGGER),
	},
};
