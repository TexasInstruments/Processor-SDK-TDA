/*
 * System Firmware Source File
 *
 * Static IA IRQ maps for Processing Entities for J7200 device
 *
 * Data version: 220622_142531
 *
 * Copyright (C) 2019-2025, Texas Instruments Incorporated
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
		 * For: A72_0 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0000U,
		.vint		= 0x0000U,
	},
	[1] =  {
		/*
		 * For: A72_0 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0001U,
		.vint		= 0x0001U,
	},
	[2] =  {
		/*
		 * For: A72_1 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0002U,
		.vint		= 0x0002U,
	},
	[3] =  {
		/*
		 * For: A72_1 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0003U,
		.vint		= 0x0003U,
	},
	[4] =  {
		/*
		 * For: A72_2 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0004U,
		.vint		= 0x0004U,
	},
	[5] =  {
		/*
		 * For: A72_2 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0005U,
		.vint		= 0x0005U,
	},
	[6] =  {
		/*
		 * For: A72_3 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0006U,
		.vint		= 0x0006U,
	},
	[7] =  {
		/*
		 * For: A72_3 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0007U,
		.vint		= 0x0007U,
	},
	[8] =  {
		/*
		 * For: A72_4 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0008U,
		.vint		= 0x0008U,
	},
	[9] =  {
		/*
		 * For: A72_4 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0009U,
		.vint		= 0x0009U,
	},
	[10] = {
		/*
		 * For: MAIN_0_R5_0 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000AU,
		.vint		= 0x000AU,
	},
	[11] = {
		/*
		 * For: MAIN_0_R5_0 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000BU,
		.vint		= 0x000BU,
	},
	[12] = {
		/*
		 * For: MAIN_0_R5_1 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000CU,
		.vint		= 0x000CU,
	},
	[13] = {
		/*
		 * For: MAIN_0_R5_1 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000DU,
		.vint		= 0x000DU,
	},
	[14] = {
		/*
		 * For: MAIN_0_R5_2 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000EU,
		.vint		= 0x000EU,
	},
	[15] = {
		/*
		 * For: MAIN_0_R5_2 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000FU,
		.vint		= 0x000FU,
	},
	[16] = {
		/*
		 * For: MAIN_0_R5_3 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0010U,
		.vint		= 0x0010U,
	},
	[17] = {
		/*
		 * For: MAIN_0_R5_3 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0011U,
		.vint		= 0x0011U,
	},
	[18] = {
		/*
		 * For: A72_5 notify
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x11FFU,
		.vint		= 0x00FFU,
	},
	[19] = {
		/*
		 * For: A72_5 response
		 */
		.id		= J7200_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x11FEU,
		.vint		= 0x00FEU,
	},
	[20] = {
		/*
		 * For: MCU_0_R5_0 notify
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4000U,
		.vint		= 0x0000U,
	},
	[21] = {
		/*
		 * For: MCU_0_R5_0 response
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4001U,
		.vint		= 0x0001U,
	},
	[22] = {
		/*
		 * For: MCU_0_R5_1 notify
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4002U,
		.vint		= 0x0002U,
	},
	[23] = {
		/*
		 * For: MCU_0_R5_1 response
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4003U,
		.vint		= 0x0003U,
	},
	[24] = {
		/*
		 * For: MCU_0_R5_2 notify
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4004U,
		.vint		= 0x0004U,
	},
	[25] = {
		/*
		 * For: MCU_0_R5_2 response
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4005U,
		.vint		= 0x0005U,
	},
	[26] = {
		/*
		 * For: MCU_0_R5_3 notify
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4006U,
		.vint		= 0x0006U,
	},
	[27] = {
		/*
		 * For: MCU_0_R5_3 response
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4007U,
		.vint		= 0x0007U,
	},
	[28] = {
		/*
		 * For: DM err_event
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4008U,
		.vint		= 0x0008U,
	},
	[29] = {
		/*
		 * For: DM nonsec_high_priority_rx
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4009U,
		.vint		= 0x0009U,
	},
	[30] = {
		/*
		 * For: DM nonsec_high_priority_rx
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400AU,
		.vint		= 0x000AU,
	},
	[31] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400BU,
		.vint		= 0x000BU,
	},
	[32] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400CU,
		.vint		= 0x000CU,
	},
	[33] = {
		/*
		 * For: DM nonsec_notify_resp_rx
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400DU,
		.vint		= 0x000DU,
	},
	[34] = {
		/*
		 * For: DM nonsec_notify_resp_rx
		 */
		.id		= J7200_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400EU,
		.vint		= 0x000EU,
	},
};
