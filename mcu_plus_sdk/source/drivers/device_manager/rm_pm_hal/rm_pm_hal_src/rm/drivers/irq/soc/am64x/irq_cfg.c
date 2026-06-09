/*
 * Interrupt Management
 *
 * Interrupt management driver instance data definition
 *
 * Data version: 230209_212853
 *
 * Copyright (C) 2019-2026, Texas Instruments Incorporated
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
#include <devices.h>
#include <devices_internal.h>
#include <hosts.h>
#include <hosts_internal.h>
#include <soc/am64x/resasg_types.h>

#include "../../include/irq_resasg.h"
#include <irq_cfg.h>

/*
 * SoC devices capable of sourcing global events
 */
const u16 evt_rt_srcs[2] = {
	AM64X_DEV_DMASS0_INTAGGR_0,
	AM64X_DEV_DMASS0_RINGACC_0,
};

const u32 evt_rt_srcs_count = ARRAY_SIZE(evt_rt_srcs);

const struct irq_global_event_type irq_global_event_types[IRQ_GLOBAL_EVENT_TYPES_ID_MAX] = {
	{
		.start = 0U,
		.num = 1536U,
		.utype = RESASG_UTYPE(AM64X_DEV_DMASS0_INTAGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
	},
	{
		.start = 8192U,
		.num = 128U,
		.utype = RESASG_UTYPE(AM64X_DEV_DMASS0_INTAGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_MEVT),
	},
	{
		.start = 10240U,
		.num = 256U,
		.utype = RESASG_UTYPE(AM64X_DEV_DMASS0_INTAGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_GEVT),
	},
	{
		.start = 32768U,
		.num = 32U,
		.utype = RESASG_UTYPE(AM64X_DEV_DMASS0_INTAGGR_0, RESASG_SUBTYPE_GLOBAL_EVENT_LEVT),
	},
	{
		.start = 50176U,
		.num = 136U,
		.utype = RESASG_UTYPE(AM64X_DEV_DMASS0_BCDMA_0, RESASG_SUBTYPE_GLOBAL_EVENT_TRIGGER),
	},
};
