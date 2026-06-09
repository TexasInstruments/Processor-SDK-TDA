/*
 * Device Manager - LPM interface
 *
 * Handlers for Low Power Mode implementation
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

#include <lib/ioremap.h>
#include <types/errno.h>
#include <tisci/lpm/tisci_lpm.h>
#include "lpm_handler.h"
#include "soc_ctrl_mmr.h"
#include <lib/trace.h>
#include <lib/mmr_lock.h>

/* Count of 1us delay for 10ms */
#define TIMEOUT_10MS                    10000U

static void lpm_hang_abort(void)
{
	volatile u32 a = 0x12341234U;

	while (a != 0U) {
	}
}

static void lpm_enter_partial_io_mode(void)
{
	u32 reg = 0u;
	u32 timeout = TIMEOUT_10MS;

	/* Unlock wkup_ctrl_mmr region 6 */
	mmr_unlock(WKUP_CTRL_BASE, 6);

	/* Set global wuen for WKUP IOs */
	reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
	reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
	reg = reg | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN;
	writel(reg, (WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0));

	/* Set global isoin for wakeup IOs */
	reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
	reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
	reg = reg | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL;
	writel(reg, (WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0));

	/* Wait for wu clock state to be 1 */
	while ((timeout > 0U) &&
	       (((readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0)) & WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) != WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS)) {
		--timeout;
	}
	if (timeout == 0U) {
		lpm_hang_abort();
	}

	/* Enable wakeup from CAN IO */
	writel(WKUP0_EN_CANUART_IO_DAISY_CHAIN, (WKUP_CTRL_BASE + WKUP0_EN));

	/* Ensure that PMIC EN control from SOC is selected */
	writel((WKUP0_PMCTRL_SYS_LPM_EN_PMIC | WKUP0_LPM_PMIC_OUT_EN), (WKUP_CTRL_BASE + WKUP0_PMCTRL_SYS));

	/* Program magic word */
	reg = WKUP_CANUART_MAGIC_WRD;
	writel(reg, WKUP_CTRL_BASE + WKUP_CANUART_CTRL);

	/* Set enable bit */
	reg |= WKUP_CANUART_MAGIC_WRD_LD_EN;
	writel(reg, WKUP_CTRL_BASE + WKUP_CANUART_CTRL);

	/* Wait for CAN_ONLY_IO signal to be 1 */
	while ((timeout > 0U) && ((readl(WKUP_CTRL_BASE + WKUP_CANUART_WAKE_STAT1)) != WKUP_CANUART_IO_MODE_ACTIVE)) {
		--timeout;
	}
	if (timeout == 0U) {
		lpm_hang_abort();
	}

	/* Enter Partial IO mode */
	writel((WKUP0_PMCTRL_SYS_LPM_EN_PMIC | WKUP0_LPM_PMIC_OUT_DIS), WKUP_CTRL_BASE + WKUP0_PMCTRL_SYS);

	/* If PMIC fails to suspend the system, hang abort */
	lpm_hang_abort();
}

static void lpm_get_partial_io_wake_info(struct tisci_msg_lpm_wake_reason_resp *wake_params)
{
	u32 val = readl(WKUP_CTRL_BASE + WKUP_PIN_SRC_REG);

	/* Set the defaults assuming partial I/O mode with invalid wake source */
	wake_params->wake_pin = TISCI_MSG_VALUE_LPM_WAKE_PIN_INVALID;
	wake_params->wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_INVALID;
	wake_params->mode = TISCI_MSG_VALUE_SLEEP_MODE_PARTIAL_IO;

	if ((val & WKUP_PIN_SRC_EARLY_WAKE_EVENT) == WKUP_PIN_SRC_EARLY_WAKE_EVENT) {
		/* If spurious wake is detected, update the pin and source */
		wake_params->wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_EARLY_WAKE_IPOR;
		if (((val & WKUP_PIN_SRC_MASK) >= CAN_IO_WKUP_PIN_NUM_START) && ((val & WKUP_PIN_SRC_MASK) <= CAN_IO_WKUP_PIN_NUM_END)) {
			wake_params->wake_pin = (u8) val;
		}
	} else if ((val >= CAN_IO_WKUP_PIN_NUM_START) && (val <= CAN_IO_WKUP_PIN_NUM_END)) {
		/* If pin number is valid, update the correct source and pin */
		wake_params->wake_pin = (u8) val;
		wake_params->wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_CAN_IO;
	} else if (val == WKUP_PIN_SRC_CLR) {
		/* If val is same as clear then it is not partial I/O, set the mode parameter to invalid */
		wake_params->mode = TISCI_MSG_VALUE_SLEEP_MODE_INVALID;
	}
}

static void lpm_reset_partial_io_wake_info(void)
{
	writel(WKUP_PIN_SRC_CLR, WKUP_CTRL_BASE + WKUP_PIN_SRC_REG);
}

s32 dm_prepare_sleep_handler(u32 *msg_recv)
{
	struct tisci_msg_prepare_sleep_req *req =
		(struct tisci_msg_prepare_sleep_req *) msg_recv;

	s32 ret = SUCCESS;
	u8 mode = req->mode;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_PREPARE_SLEEP);

	/* Reset the wake reason values */
	lpm_reset_partial_io_wake_info();

	if (mode == TISCI_MSG_VALUE_SLEEP_MODE_PARTIAL_IO) {
		/* Enable CANUART IO daisy chain and enter partial io mode */
		lpm_enter_partial_io_mode();
	} else {
		ret = -EFAIL;
	}

	return ret;
}

s32 dm_lpm_wake_reason_handler(u32 *msg_recv)
{
	struct tisci_msg_lpm_wake_reason_resp *resp =
		(struct tisci_msg_lpm_wake_reason_resp *) msg_recv;
	s32 ret = SUCCESS;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_WAKE_REASON);

	resp->hdr.flags = 0U;
	/* Write 0 to the timestamp value as the support to get time in sleep has not been added yet */
	resp->wake_timestamp = 0U;

	/* Update the wake reason params */
	lpm_get_partial_io_wake_info(resp);

	return ret;
}
