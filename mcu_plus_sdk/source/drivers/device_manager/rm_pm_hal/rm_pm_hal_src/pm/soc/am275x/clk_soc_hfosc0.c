/*
 * System Firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 * Copyright (C) 2024-2025, Texas Instruments Incorporated
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

#include <soc/am275x/clk_soc_hfosc0.h>
#include <types/array_size.h>
#include <lib/ioremap.h>
#include <compiler.h>

#define AM6_WKUP_CTRL_MMR       0x43000000UL
#define AM6_MAIN_DEVSTAT        0x30UL
#define AM6_MAIN_BOOTCFG        0x34UL

/*
 * System Oscillator enable control
 * HF Oscillator 0
 * 19.2, 20, 24, 25, 26, 27MHz
 */
static u32 clk_soc_hfosc0_get_freq(struct clk *clk_ptr UNUSED)
{
	const u32 freq_table[] = {
		FREQ_MHZ(19.2),
		FREQ_MHZ(20.0),
		FREQ_MHZ(24.0),
		FREQ_MHZ(25.0),
		FREQ_MHZ(26.0),
		FREQ_MHZ(27.0),
	};
	u32 ret = 0;
	u32 v;

	v = readl(AM6_WKUP_CTRL_MMR + AM6_MAIN_DEVSTAT) & 0x7U;
	if (v < ARRAY_SIZE(freq_table)) {
		ret = freq_table[v];
	}

	return ret;
}

static u32 clk_soc_hfosc0_get_state(struct clk *clk_ptr UNUSED)
{
	return CLK_HW_STATE_ENABLED;
}

const struct clk_drv clk_drv_soc_hfosc0 = {
	.get_freq	= clk_soc_hfosc0_get_freq,
	.get_state	= clk_soc_hfosc0_get_state,
};
