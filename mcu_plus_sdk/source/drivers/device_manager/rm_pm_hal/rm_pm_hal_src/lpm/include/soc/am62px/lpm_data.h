/*
 * Data version: 250904_170808
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
#ifndef SOC_AM62PX_LPM_DATA_H
#define SOC_AM62PX_LPM_DATA_H

#include <config.h>
#include <types/short_types.h>

enum {
	WAKESOURCE_WKUP_I2C0,
	WAKESOURCE_WKUP_UART0,
	WAKESOURCE_MCU_GPIO0,
	WAKESOURCE_WKUP_ICEMELTER0,
	WAKESOURCE_WKUP_TIMER0,
	WAKESOURCE_WKUP_TIMER1,
	WAKESOURCE_WKUP_RTCSS0,
	WAKESOURCE_GLUELOGIC_INVERTED_MAIN_RESETZ_LATCHED,
	WAKESOURCE_USB0,
	WAKESOURCE_USB1,
	WAKESOURCE_GLUELOGIC_IO_SWAKEUP_MAIN_IO,
	WAKESOURCE_GLUELOGIC_IO_SWAKEUP_MCU_IO,
	WAKESOURCE_GLUELOGIC_IO_SWAKEUP_CAN_USART_IO,
	WAKEUP_SOURCE_MAX,
};

#endif /* SOC_AM62PX_LPM_DATA_H */
