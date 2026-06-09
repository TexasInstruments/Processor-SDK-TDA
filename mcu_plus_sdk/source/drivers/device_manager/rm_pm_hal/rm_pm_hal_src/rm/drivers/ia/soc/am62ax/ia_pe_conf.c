/*
 * System Firmware Source File
 *
 * Static IA IRQ maps for Processing Entities for AM62AX device
 *
 * Data version: 250904_120008
 *
 * Copyright (C) 2022-2026, Texas Instruments Incorporated
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

#include <devices.h>

#include "../../include/ia_init.h"
#include "ia_pe_conf.h"

const struct ia_init ia_soc_pe_init_list[IA_SOC_PE_INIT_NUM] = {
	[0] =  {
		/*
		 * For: MAIN_0_R5_0 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0000U,
		.vint		= 0x0028U,
	},
	[1] =  {
		/*
		 * For: MAIN_0_R5_1 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0001U,
		.vint		= 0x0029U,
	},
	[2] =  {
		/*
		 * For: A53_0 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0002U,
		.vint		= 0x0000U,
	},
	[3] =  {
		/*
		 * For: A53_1 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0003U,
		.vint		= 0x0001U,
	},
	[4] =  {
		/*
		 * For: A53_2 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0004U,
		.vint		= 0x0002U,
	},
	[5] =  {
		/*
		 * For: A53_3 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0005U,
		.vint		= 0x0003U,
	},
	[6] =  {
		/*
		 * For: MCU_0_R5_0 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x05FFU,
		.vint		= 0x00B7U,
	},
	[7] =  {
		/*
		 * For: A53_4 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0006U,
		.vint		= 0x0004U,
	},
	[8] =  {
		/*
		 * For: C7X_0_0 response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x0007U,
		.vint		= 0x0005U,
	},
	[9] =  {
		/*
		 * For: DM2TIFS response
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x05FEU,
		.vint		= 0x004FU,
	},
	[10] = {
		/*
		 * For: DM err_event
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x000CU,
		.vint		= 0x002AU,
	},
	[11] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= AM62AX_DEV_DMASS0_INTAGGR_0,
		.event_id	= 0x000DU,
		.vint		= 0x002BU,
	},
};
