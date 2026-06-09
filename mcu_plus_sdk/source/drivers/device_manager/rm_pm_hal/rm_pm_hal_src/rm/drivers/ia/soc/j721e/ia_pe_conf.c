/*
 * System Firmware Source File
 *
 * Static IA IRQ maps for Processing Entities for J721E device
 *
 * Data version: 210812_211103
 *
 * Copyright (C) 2018-2025, Texas Instruments Incorporated
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
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0000U,
		.vint		= 0x0000U,
	},
	[1] =  {
		/*
		 * For: A72_0 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0001U,
		.vint		= 0x0001U,
	},
	[2] =  {
		/*
		 * For: A72_1 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0002U,
		.vint		= 0x0002U,
	},
	[3] =  {
		/*
		 * For: A72_1 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0003U,
		.vint		= 0x0003U,
	},
	[4] =  {
		/*
		 * For: A72_2 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0004U,
		.vint		= 0x0004U,
	},
	[5] =  {
		/*
		 * For: A72_2 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0005U,
		.vint		= 0x0005U,
	},
	[6] =  {
		/*
		 * For: A72_3 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0006U,
		.vint		= 0x0006U,
	},
	[7] =  {
		/*
		 * For: A72_3 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0007U,
		.vint		= 0x0007U,
	},
	[8] =  {
		/*
		 * For: A72_4 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0008U,
		.vint		= 0x0008U,
	},
	[9] =  {
		/*
		 * For: A72_4 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0009U,
		.vint		= 0x0009U,
	},
	[10] = {
		/*
		 * For: C7X_0 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x000AU,
		.vint		= 0x000AU,
	},
	[11] = {
		/*
		 * For: C7X_0 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x000BU,
		.vint		= 0x000BU,
	},
	[12] = {
		/*
		 * For: C7X_1 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x000CU,
		.vint		= 0x000CU,
	},
	[13] = {
		/*
		 * For: C7X_1 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x000DU,
		.vint		= 0x000DU,
	},
	[14] = {
		/*
		 * For: C6X_0_0 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x000EU,
		.vint		= 0x000EU,
	},
	[15] = {
		/*
		 * For: C6X_0_0 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x000FU,
		.vint		= 0x000FU,
	},
	[16] = {
		/*
		 * For: C6X_0_1 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0010U,
		.vint		= 0x0010U,
	},
	[17] = {
		/*
		 * For: C6X_0_1 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0011U,
		.vint		= 0x0011U,
	},
	[18] = {
		/*
		 * For: C6X_1_0 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0012U,
		.vint		= 0x0012U,
	},
	[19] = {
		/*
		 * For: C6X_1_0 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0013U,
		.vint		= 0x0013U,
	},
	[20] = {
		/*
		 * For: C6X_1_1 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0014U,
		.vint		= 0x0014U,
	},
	[21] = {
		/*
		 * For: C6X_1_1 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0015U,
		.vint		= 0x0015U,
	},
	[22] = {
		/*
		 * For: MAIN_0_R5_0 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0016U,
		.vint		= 0x0016U,
	},
	[23] = {
		/*
		 * For: MAIN_0_R5_0 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0017U,
		.vint		= 0x0017U,
	},
	[24] = {
		/*
		 * For: MAIN_0_R5_1 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0018U,
		.vint		= 0x0018U,
	},
	[25] = {
		/*
		 * For: MAIN_0_R5_1 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0019U,
		.vint		= 0x0019U,
	},
	[26] = {
		/*
		 * For: MAIN_0_R5_2 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x001AU,
		.vint		= 0x001AU,
	},
	[27] = {
		/*
		 * For: MAIN_0_R5_2 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x001BU,
		.vint		= 0x001BU,
	},
	[28] = {
		/*
		 * For: MAIN_0_R5_3 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x001CU,
		.vint		= 0x001CU,
	},
	[29] = {
		/*
		 * For: MAIN_0_R5_3 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x001DU,
		.vint		= 0x001DU,
	},
	[30] = {
		/*
		 * For: MAIN_1_R5_0 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x001EU,
		.vint		= 0x001EU,
	},
	[31] = {
		/*
		 * For: MAIN_1_R5_0 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x001FU,
		.vint		= 0x001FU,
	},
	[32] = {
		/*
		 * For: MAIN_1_R5_1 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0020U,
		.vint		= 0x0020U,
	},
	[33] = {
		/*
		 * For: MAIN_1_R5_1 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0021U,
		.vint		= 0x0021U,
	},
	[34] = {
		/*
		 * For: MAIN_1_R5_2 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0022U,
		.vint		= 0x0022U,
	},
	[35] = {
		/*
		 * For: MAIN_1_R5_2 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0023U,
		.vint		= 0x0023U,
	},
	[36] = {
		/*
		 * For: MAIN_1_R5_3 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0024U,
		.vint		= 0x0024U,
	},
	[37] = {
		/*
		 * For: MAIN_1_R5_3 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x0025U,
		.vint		= 0x0025U,
	},
	[38] = {
		/*
		 * For: A72_5 notify
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x11FFU,
		.vint		= 0x00FFU,
	},
	[39] = {
		/*
		 * For: A72_5 response
		 */
		.id		= J721E_DEV_NAVSS0_UDMASS_INTAGGR_0,
		.event_id	= 0x11FEU,
		.vint		= 0x00FEU,
	},
	[40] = {
		/*
		 * For: MCU_0_R5_0 notify
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4000U,
		.vint		= 0x0000U,
	},
	[41] = {
		/*
		 * For: MCU_0_R5_0 response
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4001U,
		.vint		= 0x0001U,
	},
	[42] = {
		/*
		 * For: MCU_0_R5_1 notify
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4002U,
		.vint		= 0x0002U,
	},
	[43] = {
		/*
		 * For: MCU_0_R5_1 response
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4003U,
		.vint		= 0x0003U,
	},
	[44] = {
		/*
		 * For: MCU_0_R5_2 notify
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4004U,
		.vint		= 0x0004U,
	},
	[45] = {
		/*
		 * For: MCU_0_R5_2 response
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4005U,
		.vint		= 0x0005U,
	},
	[46] = {
		/*
		 * For: MCU_0_R5_3 notify
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4006U,
		.vint		= 0x0006U,
	},
	[47] = {
		/*
		 * For: MCU_0_R5_3 response
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4007U,
		.vint		= 0x0007U,
	},
	[48] = {
		/*
		 * For: DM err_event
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4008U,
		.vint		= 0x0008U,
	},
	[49] = {
		/*
		 * For: DM nonsec_high_priority_rx
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4009U,
		.vint		= 0x0009U,
	},
	[50] = {
		/*
		 * For: DM nonsec_high_priority_rx
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400AU,
		.vint		= 0x000AU,
	},
	[51] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400BU,
		.vint		= 0x000BU,
	},
	[52] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400CU,
		.vint		= 0x000CU,
	},
	[53] = {
		/*
		 * For: DM nonsec_notify_resp_rx
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400DU,
		.vint		= 0x000DU,
	},
	[54] = {
		/*
		 * For: DM nonsec_notify_resp_rx
		 */
		.id		= J721E_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x400EU,
		.vint		= 0x000EU,
	},
};
