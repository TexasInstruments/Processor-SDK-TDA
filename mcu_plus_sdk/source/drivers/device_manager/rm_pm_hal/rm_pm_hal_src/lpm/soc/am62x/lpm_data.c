/*
 * Data version: 250903_173617
 *
 * Copyright (C) 2021-2026, Texas Instruments Incorporated
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
#include <config.h>
#include <lpm_data.h>
#include <wake_data.h>
#include <types/short_types.h>
#include <tisci/lpm/tisci_lpm.h>

const struct wake_source_data soc_wake_sources_data[WAKEUP_SOURCE_MAX] = {
	[WAKESOURCE_WKUP_I2C0] =			      {
		.wkup_idx	= 0,
		.int_num	= 143,
		.source		= "WKUP_I2C0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_WKUP_I2C0,
	},
	[WAKESOURCE_WKUP_UART0] =			      {
		.wkup_idx	= 1,
		.int_num	= 144,
		.source		= "WKUP_UART0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_WKUP_UART0,
	},
	[WAKESOURCE_MCU_GPIO0] =			      {
		.wkup_idx	= 2,
		.int_num	= 18,
		.source		= "MCU_GPIO0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_MCU_GPIO0,
	},
	[WAKESOURCE_WKUP_ICEMELTER0] =			      {
		.wkup_idx	= 3,
		.int_num	= 23,
		.source		= "WKUP_ICEMELTER0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_WKUP_ICEMELTER0,
	},
	[WAKESOURCE_WKUP_TIMER0] =			      {
		.wkup_idx	= 5,
		.int_num	= 28,
		.source		= "WKUP_TIMER0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_WKUP_TIMER0,
	},
	[WAKESOURCE_WKUP_TIMER1] =			      {
		.wkup_idx	= 6,
		.int_num	= 29,
		.source		= "WKUP_TIMER1",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_WKUP_TIMER1,
	},
	[WAKESOURCE_WKUP_RTCSS0] =			      {
		.wkup_idx	= 7,
		.int_num	= 97,
		.source		= "WKUP_RTCSS0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_WKUP_RTC0,
	},
	[WAKESOURCE_GLUELOGIC_INVERTED_MAIN_RESETZ_LATCHED] = {
		.wkup_idx	= 8,
		.int_num	= 26,
		.source		= "GLUELOGIC_INVERTED_MAIN_RESETZ_LATCHED",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_RESET,
	},
	[WAKESOURCE_USB0] =				      {
		.wkup_idx	= 9,
		.int_num	= 61,
		.source		= "USB0",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_USB0,
	},
	[WAKESOURCE_USB1] =				      {
		.wkup_idx	= 10,
		.int_num	= 62,
		.source		= "USB1",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_USB1,
	},
	[WAKESOURCE_GLUELOGIC_IO_SWAKEUP_MAIN_IO] =	      {
		.wkup_idx	= 16,
		.int_num	= 252,
		.source		= "GLUELOGIC_IO_SWAKEUP_MAIN_IO",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_MAIN_IO,
	},
	[WAKESOURCE_GLUELOGIC_IO_SWAKEUP_MCU_IO] =	      {
		.wkup_idx	= 17,
		.int_num	= 253,
		.source		= "GLUELOGIC_IO_SWAKEUP_MCU_IO",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_MCU_IO,
	},
	[WAKESOURCE_GLUELOGIC_IO_SWAKEUP_CAN_USART_IO] =      {
		.wkup_idx	= 18,
		.int_num	= 251,
		.source		= "GLUELOGIC_IO_SWAKEUP_CAN_USART_IO",
		.source_id	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_CAN_IO,
	},
};
