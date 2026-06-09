/*
 * DMSC firmware
 *
 * Cortex-M4 (CM4) firmware for power management
 *
 * Copyright (C) 2025, Texas Instruments Incorporated
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

#include <hosts.h>
#include <types/address_types.h>
#include <tisci/lpm/tisci_lpm.h>
#include <stdbool.h>
#include <boardcfg/pm_boardcfg_validate.h>

const u8 valid_hosts[] = {
	HOST_ID_MCU_0_R5_0,
	HOST_ID_MCU_0_R5_1,
	HOST_ID_MCU_0_R5_2,
	HOST_ID_MCU_0_R5_3,
	HOST_ID_A72_0,
	HOST_ID_A72_1,
	HOST_ID_A72_2,
	HOST_ID_A72_3,
	HOST_ID_A72_4,
	HOST_ID_A72_5,
	HOST_ID_A72_6,
	HOST_ID_A72_7,
	HOST_ID_MAIN_0_R5_0,
	HOST_ID_MAIN_0_R5_1,
	HOST_ID_MAIN_0_R5_2,
	HOST_ID_MAIN_0_R5_3,
	HOST_ID_MAIN_1_R5_0,
	HOST_ID_MAIN_1_R5_1,
	HOST_ID_MAIN_1_R5_2,
	HOST_ID_MAIN_1_R5_3,
	HOST_ID_MAIN_2_R5_0,
	HOST_ID_MAIN_2_R5_1,
	HOST_ID_MAIN_2_R5_2,
	HOST_ID_MAIN_2_R5_3,
};

const u8 lpm_modes[] = {
	TISCI_MSG_VALUE_SLEEP_MODE_SOC_OFF,
	TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR,
};

bool is_suspend_initiator_valid(u8 initiator)
{
	bool ret = false;
	int i;

	for (i = 0; i < sizeof(valid_hosts); i++) {
		if (initiator == valid_hosts[i]) {
			ret = true;
			break;
		}
	}

	return ret;
}

bool is_lpm_mode_valid(u8 lpm_mode)
{
	bool ret = false;
	int i;

	for (i = 0; i < sizeof(lpm_modes); i++) {
		if (lpm_mode == lpm_modes[i]) {
			ret = true;
			break;
		}
	}

	return ret;
}
