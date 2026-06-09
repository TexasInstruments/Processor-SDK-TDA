/*
 * System Firmware Source File
 *
 * Static IA IRQ maps for Processing Entities for J784S4 device
 *
 * Data version: 250223_004013
 *
 * Copyright (C) 2022-2025, Texas Instruments Incorporated
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
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0000U,
		.vint		= 0x0000U,
	},
	[1] =  {
		/*
		 * For: A72_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0001U,
		.vint		= 0x0001U,
	},
	[2] =  {
		/*
		 * For: A72_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0002U,
		.vint		= 0x0002U,
	},
	[3] =  {
		/*
		 * For: A72_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0003U,
		.vint		= 0x0003U,
	},
	[4] =  {
		/*
		 * For: A72_2 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0004U,
		.vint		= 0x0004U,
	},
	[5] =  {
		/*
		 * For: A72_2 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0005U,
		.vint		= 0x0005U,
	},
	[6] =  {
		/*
		 * For: A72_3 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0006U,
		.vint		= 0x0006U,
	},
	[7] =  {
		/*
		 * For: A72_3 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0007U,
		.vint		= 0x0007U,
	},
	[8] =  {
		/*
		 * For: A72_4 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0008U,
		.vint		= 0x0008U,
	},
	[9] =  {
		/*
		 * For: A72_4 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0009U,
		.vint		= 0x0009U,
	},
	[10] = {
		/*
		 * For: A72_5 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000AU,
		.vint		= 0x000AU,
	},
	[11] = {
		/*
		 * For: A72_5 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000BU,
		.vint		= 0x000BU,
	},
	[12] = {
		/*
		 * For: A72_6 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000CU,
		.vint		= 0x000CU,
	},
	[13] = {
		/*
		 * For: A72_6 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000DU,
		.vint		= 0x000DU,
	},
	[14] = {
		/*
		 * For: A72_7 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000EU,
		.vint		= 0x000EU,
	},
	[15] = {
		/*
		 * For: A72_7 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x000FU,
		.vint		= 0x000FU,
	},
	[16] = {
		/*
		 * For: C7X_0_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0010U,
		.vint		= 0x0010U,
	},
	[17] = {
		/*
		 * For: C7X_0_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0011U,
		.vint		= 0x0011U,
	},
	[18] = {
		/*
		 * For: C7X_0_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0012U,
		.vint		= 0x0012U,
	},
	[19] = {
		/*
		 * For: C7X_0_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0013U,
		.vint		= 0x0013U,
	},
	[20] = {
		/*
		 * For: C7X_1_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0014U,
		.vint		= 0x0014U,
	},
	[21] = {
		/*
		 * For: C7X_1_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0015U,
		.vint		= 0x0015U,
	},
	[22] = {
		/*
		 * For: C7X_1_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0016U,
		.vint		= 0x0016U,
	},
	[23] = {
		/*
		 * For: C7X_1_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0017U,
		.vint		= 0x0017U,
	},
	[24] = {
		/*
		 * For: C7X_2_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0018U,
		.vint		= 0x0018U,
	},
	[25] = {
		/*
		 * For: C7X_2_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0019U,
		.vint		= 0x0019U,
	},
	[26] = {
		/*
		 * For: C7X_2_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x001AU,
		.vint		= 0x001AU,
	},
	[27] = {
		/*
		 * For: C7X_2_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x001BU,
		.vint		= 0x001BU,
	},
	[28] = {
		/*
		 * For: C7X_3_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x001CU,
		.vint		= 0x001CU,
	},
	[29] = {
		/*
		 * For: C7X_3_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x001DU,
		.vint		= 0x001DU,
	},
	[30] = {
		/*
		 * For: C7X_3_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x001EU,
		.vint		= 0x001EU,
	},
	[31] = {
		/*
		 * For: C7X_3_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x001FU,
		.vint		= 0x001FU,
	},
	[32] = {
		/*
		 * For: MAIN_0_R5_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0020U,
		.vint		= 0x0020U,
	},
	[33] = {
		/*
		 * For: MAIN_0_R5_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0021U,
		.vint		= 0x0021U,
	},
	[34] = {
		/*
		 * For: MAIN_0_R5_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0022U,
		.vint		= 0x0022U,
	},
	[35] = {
		/*
		 * For: MAIN_0_R5_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0023U,
		.vint		= 0x0023U,
	},
	[36] = {
		/*
		 * For: MAIN_0_R5_2 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0024U,
		.vint		= 0x0024U,
	},
	[37] = {
		/*
		 * For: MAIN_0_R5_2 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0025U,
		.vint		= 0x0025U,
	},
	[38] = {
		/*
		 * For: MAIN_0_R5_3 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0026U,
		.vint		= 0x0026U,
	},
	[39] = {
		/*
		 * For: MAIN_0_R5_3 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0027U,
		.vint		= 0x0027U,
	},
	[40] = {
		/*
		 * For: MAIN_1_R5_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0028U,
		.vint		= 0x0028U,
	},
	[41] = {
		/*
		 * For: MAIN_1_R5_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0029U,
		.vint		= 0x0029U,
	},
	[42] = {
		/*
		 * For: MAIN_1_R5_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x002AU,
		.vint		= 0x002AU,
	},
	[43] = {
		/*
		 * For: MAIN_1_R5_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x002BU,
		.vint		= 0x002BU,
	},
	[44] = {
		/*
		 * For: MAIN_1_R5_2 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x002CU,
		.vint		= 0x002CU,
	},
	[45] = {
		/*
		 * For: MAIN_1_R5_2 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x002DU,
		.vint		= 0x002DU,
	},
	[46] = {
		/*
		 * For: MAIN_1_R5_3 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x002EU,
		.vint		= 0x002EU,
	},
	[47] = {
		/*
		 * For: MAIN_1_R5_3 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x002FU,
		.vint		= 0x002FU,
	},
	[48] = {
		/*
		 * For: MAIN_2_R5_0 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0030U,
		.vint		= 0x0030U,
	},
	[49] = {
		/*
		 * For: MAIN_2_R5_0 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0031U,
		.vint		= 0x0031U,
	},
	[50] = {
		/*
		 * For: MAIN_2_R5_1 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0032U,
		.vint		= 0x0032U,
	},
	[51] = {
		/*
		 * For: MAIN_2_R5_1 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0033U,
		.vint		= 0x0033U,
	},
	[52] = {
		/*
		 * For: MAIN_2_R5_2 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0034U,
		.vint		= 0x0034U,
	},
	[53] = {
		/*
		 * For: MAIN_2_R5_2 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0035U,
		.vint		= 0x0035U,
	},
	[54] = {
		/*
		 * For: MAIN_2_R5_3 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0036U,
		.vint		= 0x0036U,
	},
	[55] = {
		/*
		 * For: MAIN_2_R5_3 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x0037U,
		.vint		= 0x0037U,
	},
	[56] = {
		/*
		 * For: A72_8 notify
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x11FFU,
		.vint		= 0x00FFU,
	},
	[57] = {
		/*
		 * For: A72_8 response
		 */
		.id		= J784S4_DEV_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x11FEU,
		.vint		= 0x00FEU,
	},
	[58] = {
		/*
		 * For: MCU_0_R5_0 notify
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4000U,
		.vint		= 0x0000U,
	},
	[59] = {
		/*
		 * For: MCU_0_R5_0 response
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4001U,
		.vint		= 0x0001U,
	},
	[60] = {
		/*
		 * For: MCU_0_R5_1 notify
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4002U,
		.vint		= 0x0002U,
	},
	[61] = {
		/*
		 * For: MCU_0_R5_1 response
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4003U,
		.vint		= 0x0003U,
	},
	[62] = {
		/*
		 * For: MCU_0_R5_2 notify
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4004U,
		.vint		= 0x0004U,
	},
	[63] = {
		/*
		 * For: MCU_0_R5_2 response
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4005U,
		.vint		= 0x0005U,
	},
	[64] = {
		/*
		 * For: MCU_0_R5_3 notify
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4006U,
		.vint		= 0x0006U,
	},
	[65] = {
		/*
		 * For: MCU_0_R5_3 response
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4007U,
		.vint		= 0x0007U,
	},
	[66] = {
		/*
		 * For: DM err_event
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4010U,
		.vint		= 0x0010U,
	},
	[67] = {
		/*
		 * For: DM nonsec_high_priority_rx
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4011U,
		.vint		= 0x0011U,
	},
	[68] = {
		/*
		 * For: DM nonsec_high_priority_rx
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4012U,
		.vint		= 0x0012U,
	},
	[69] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4013U,
		.vint		= 0x0013U,
	},
	[70] = {
		/*
		 * For: DM nonsec_low_priority_rx
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4014U,
		.vint		= 0x0014U,
	},
	[71] = {
		/*
		 * For: DM nonsec_notify_resp_rx
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4015U,
		.vint		= 0x0015U,
	},
	[72] = {
		/*
		 * For: DM nonsec_notify_resp_rx
		 */
		.id		= J784S4_DEV_MCU_NAVSS0_UDMASS_INTA_0,
		.event_id	= 0x4016U,
		.vint		= 0x0016U,
	},
};
