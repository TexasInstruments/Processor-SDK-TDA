/*
 * Device Manager - LPM interface
 *
 * Handlers for Low Power Mode implementation
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

#include <lib/ioremap.h>
#include <types/errno.h>
#include <string.h>
#include <tisci/lpm/tisci_lpm.h>
#include <osal/osal_clock_user.h>
#include <osal_hwi.h>
#include <osal_dm.h>
#include "gtc.h"
#include "lpm_handler.h"
#include "padcfg.h"
#include "device.h"
#include "device_pm.h"
#include "devices.h"
#include "sec_proxy.h"
#include "clk.h"
#include "soc_ctrl_mmr.h"
#include "ctrlmmr_raw.h"
#include "ctrl_mmr.h"
#include "rm_lpm.h"
#include "device_prepare.h"
#include <soc/host_idx_mapping.h>
#include <lib/trace.h>
#include "wkup_periph.h"
#include "rm.h"

/* Count of 1us delay for 10ms */
#define TIMEOUT_10MS                    10000U

/* DM stub + boot code present in TCM */
#define TCMB_SIZE                       0x8000U

#define LPM_SUSPEND_POWERMASTER                 BIT(0)
#define LPM_DEVICE_DEINIT                       BIT(1)
#define LPM_SUSPEND_DM_APPLICATION              BIT(2)
#define LPM_SAVE_MAIN_PADCONFIG                 BIT(3)
#define LPM_SUSPEND_GTC                         BIT(4)
#define LPM_CLOCK_SUSPEND                       BIT(5)
#define LPM_SUSPEND_DM                          BIT(6)
#define LPM_SAVE_MMR_LOCK                       BIT(7)
#define LPM_SAVE_MCU_PADCONFIG                  BIT(8)
#define LPM_SAVE_WKUP_PERIPH_CFG                BIT(9)
#define LPM_RM_DEINIT                           BIT(10)

#define LPM_RESUME_LATENCY_VALID_FLAG           BIT(16)

/* IO DDR, Deep sleep and MCU Only latency values */
#define LPM_IO_ONLY_PLUS_DDR_RESUME_LAT_MIN     251U
#define LPM_DEEP_SLEEP_RESUME_LAT_MIN           101U
#define LPM_MCU_ONLY_RESUME_LAT_MIN             10U

#define MIN(a, b)                               ((a > b) ? b : a)

extern s32 _stub_start(void);
extern void lpm_get_wake_info(struct tisci_msg_lpm_wake_reason_resp *wkup_params);
extern void lpm_populate_prepare_sleep_data(struct tisci_msg_prepare_sleep_req *p);
extern void lpm_clear_all_wakeup_interrupt(void);
extern u8 lpm_get_selected_sleep_mode(void);
extern void lpm_reset_wake_reason_params(void);
extern s32 unload_magic_words_thru_wkup_mmr(void);

u32 key_status;
volatile u32 enter_sleep_status = 0;

u8 dm_stub_arr[TCMB_SIZE] = { 0U };

/* Each bit represents whether that host has set constraints or not */
#if HOST_ID_CNT <= 16U
static u16 dev_cons[SOC_DEVICES_RANGE_ID_MAX] = { 0U };
#elif HOST_ID_CNT <= 32U
static u32 dev_cons[SOC_DEVICES_RANGE_ID_MAX] = { 0U };
#else
static u64 dev_cons[SOC_DEVICES_RANGE_ID_MAX] = { 0U };
#endif

static u32 latency[HOST_ID_CNT] = { 0U };
static sbool lpm_locked = SFALSE;

static void lpm_clear_sleep_data(void)
{
	u32 idx;

	/* Clear all device constraints */
	for (idx = 0U; idx < SOC_DEVICES_RANGE_ID_MAX; idx++) {
		dev_cons[idx] = 0U;
	}

	/* Clear all latency constraints */
	for (idx = 0U; idx < HOST_ID_CNT; idx++) {
		latency[idx] = 0U;
	}

	/* Open the mode selection lock */
	lpm_locked = SFALSE;
}

static u8 lpm_select_shallowest_mode(u8 req_mode, u8 curr_mode)
{
	u8 mode;

	if ((curr_mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY) || (req_mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
		mode = TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY;
	} else if ((curr_mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (req_mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP)) {
		mode = TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP;
	} else {
		mode = DEEPEST_LOW_POWER_MODE;
	}

	return mode;
}

static void lpm_hang_abort(void)
{
	volatile u32 a = 0x12341234;

	while (a != 0U) {
	}
}

static s32 lpm_sleep_wait_for_tifs_wfi(void)
{
	u32 reg;
	u32 i = 0;
	s32 ret = -ETIMEDOUT;

	do {
		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_WFI_STATUS);
		if ((reg & SMS_CPU0_WFI_MASK) == SMS_CPU0_WFI_MASK) {
			ret = SUCCESS;
			break;
		}
		osal_delay(1);
	} while (i++ < TIMEOUT_10MS);

	return ret;
}

static s32 lpm_resume_restore_RM_context(void)
{
	/* Restore IR configurations */
	s32 ret = -EFAIL;

	ret = rm_init();

	if (ret == SUCCESS) {
		ret = lpm_resume_restore_ir_config();
	}

	return ret;
}

static s32 lpm_suspend_RM_context(void)
{
	/* Clear all initialized as well as ROM usage clear flags so that ROM resources can be freed during resume */
	s32 ret = -EFAIL;

	/* Deinit all RM resources */
	ret = rm_deinit(RM_RSRC_DEVGRP, STRUE);

	return ret;
}

static s32 lpm_resume_send_core_resume_message(void)
{
	/* Send core resume message */
	s32 ret = 0;

	struct tisci_msg_core_resume_req req = {
		.hdr		= {
			.type	= TISCI_MSG_CORE_RESUME,
			.flags	= TISCI_MSG_FLAG_AOP,
			.host	= HOST_ID_DM2TIFS
		}
	};

	ret = sproxy_send_msg_dm2dmsc_fw(&req, sizeof(req));

	if (ret == 0) {
		struct tisci_msg_core_resume_resp resp;
		memset(&resp, 0, sizeof(resp));

		ret = sproxy_receive_msg_dm2dmsc_fw(&resp, sizeof(resp));
		if (ret == 0) {
			if ((resp.hdr.type != TISCI_MSG_CORE_RESUME) || ((resp.hdr.flags & TISCI_MSG_FLAG_ACK) != TISCI_MSG_FLAG_ACK)) {
				ret = -EINVAL;
			}
		}
	}

	return ret;
}

static s32 lpm_resume_send_enter_sleep_abort_message(void)
{
	/* Send abort enter sleep message */
	s32 ret = 0;

	struct tisci_msg_dm_abort_sleep_req req = {
		.hdr		= {
			.type	= TISCI_MSG_DM_ABORT_SLEEP,
			.flags	= 0,
			.host	= HOST_ID_DM2TIFS
		}
	};

	ret = sproxy_send_msg_dm2dmsc_fw(&req, sizeof(req));

	return ret;
}

static s32 lpm_suspend_power_master(void)
{
	/* Release reset of power master */
	struct device *dev;

	dev = device_lookup(DEV_GTC);
	soc_device_ret_disable(dev);
	soc_device_disable(dev, SFALSE);

	dev = device_lookup(POWER_MASTER);
	soc_device_ret_disable(dev);
	soc_device_disable(dev, SFALSE);

	dev = device_lookup(POWER_MASTER_CLUSTER);
	soc_device_ret_disable(dev);
	soc_device_disable(dev, SFALSE);

	return SUCCESS;
}

static s32 lpm_resume_release_reset_of_power_master(void)
{
	/* Release reset of power master */
	struct device *dev;

	dev = device_lookup(POWER_MASTER_CLUSTER);
	soc_device_enable(dev);

	dev = device_lookup(POWER_MASTER);
	soc_device_enable(dev);

	dev = device_lookup(DEV_GTC);
	soc_device_enable(dev);

	return SUCCESS;
}

static s32 lpm_sleep_suspend_dm(void)
{
	/* Suspend DM OS */
	osal_dm_disable_interrupt();            /* Disable sciserver interrupt */
	osal_suspend_dm();                      /* Suspend DM task scheduler */
	key_status = osal_hwip_disable();       /* Disable Global interrupt */
	return SUCCESS;
}

static s32 lpm_resume_dm(void)
{
	/* Resume DM OS */
	osal_resume_dm();               /* Resume DM task scheduler */
	osal_hwip_restore(key_status);  /* Enable Global interrupts */
	return SUCCESS;
}

static s32 lpm_copy_fs_stub(void)
{
	return osal_dm_copy_fs_stub_from_ddr_to_local_mem();
}

static s32 lpm_sleep_jump_to_dm_Stub(void)
{
	/* Jump to DM stub */
	return _stub_start();
}

static s32 lpm_select_sleep_mode(u8 *mode)
{
	u8 lpsc;
	u16 devgrp;
	u8 i = 0U;
	s32 ret = SUCCESS;
	sbool is_shallowest_mode_selected = SFALSE;
	sbool main_padcfg_wkup_en = SFALSE;
	sbool mcu_padcfg_wkup_en = SFALSE;
	sbool done = SFALSE;

	*mode = DEEPEST_LOW_POWER_MODE;

	/* Device constraint based selection */
	for (i = 0; i < soc_device_count; i++) {
		if (dev_cons[i] != 0U) {
			/* Get devgrp and LPSC info from device_id */
			ret = device_id_lookup_devgroup_and_lpsc(i, &devgrp, &lpsc);
			if (ret != SUCCESS) {
				ret = -EFAIL;
				done = STRUE;
			}

			if (done == SFALSE) {
				/**
				 * If device having constraints belongs to MAIN DEVGRP, then no lpm is possible
				 * Exceptions: USB0 and USB1
				 */
				if (devgrp == MAIN_DEVGRP) {
					if ((i == USB0_DEV_ID) || (i == USB1_DEV_ID) || (i == USB0_ISO_DEV_ID) || (i == USB1_ISO_DEV_ID)) {
						*mode = lpm_select_shallowest_mode(TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP, *mode);
					} else {
						ret = -EFAIL;
						done = STRUE;
					}
				} else {
					/* If MCU Only mode has not been already selected */
					if (is_shallowest_mode_selected == SFALSE) {
						/* If device is in ALWAYS ON domain in MCU_WAKEUP devgrp, then select Deep sleep */
						if (lpsc == ALWAYS_ON_LPSC_ID) {
							*mode = lpm_select_shallowest_mode(TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP, *mode);
							/* Otherwise, the device belongs to MCU Domain, select MCU only mode */
						} else {
							*mode = TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY;
							is_shallowest_mode_selected = STRUE;
						}
					}
				}
			}
		}
		if (done == STRUE) {
			break;
		}
	}

	/* Latency based selection */
	if (ret == SUCCESS) {
		done = SFALSE;
		for (i = 0; i < HOST_ID_CNT; i++) {
			if ((latency[i] & LPM_RESUME_LATENCY_VALID_FLAG) != 0U) {
				/* If the latency value is more than or equal to deepest mode minimum resume latency, select deepest lpm mode */
				if ((u16) latency[i] >= LPM_IO_ONLY_PLUS_DDR_RESUME_LAT_MIN) {
					*mode = lpm_select_shallowest_mode(DEEPEST_LOW_POWER_MODE, *mode);
					/* If the latency value is more than or equal to deep sleep mode minimum resume latency, select deep sleep */
				} else if ((u16) latency[i] >= LPM_DEEP_SLEEP_RESUME_LAT_MIN) {
					*mode = lpm_select_shallowest_mode(TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP, *mode);
					/* If the latency value lie in mcu only mode resume latency range, select mcu only */
				} else if ((u16) latency[i] >= LPM_MCU_ONLY_RESUME_LAT_MIN) {
					*mode = TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY;
					is_shallowest_mode_selected = STRUE;
					/* If the latency value is out of resume latency range values, no lpm is possible */
				} else {
					ret = -EFAIL;
					done = STRUE;
				}
			}
			if (done == STRUE) {
				break;
			}
		}
	}

	/* Scan and save the padconfig values */
	if (ret == SUCCESS) {
		ret = lpm_sleep_save_main_padconf(&main_padcfg_wkup_en);
	}

	if (ret == SUCCESS) {
		ret = lpm_sleep_save_mcu_padconf(&mcu_padcfg_wkup_en);
	}

	if (ret == SUCCESS) {
		/*
		 * Wakeup source based selection (Padconfig) - Skipped if shallowest mode is already selected
		 * Note: In MCU Only mode, all padconfigs can act as a wakeup source
		 */
		if (((main_padcfg_wkup_en == STRUE) || (mcu_padcfg_wkup_en == STRUE)) && (is_shallowest_mode_selected == SFALSE)) {
			/* Deepest mode is selected unless explicit constraint is there */
			*mode = lpm_select_shallowest_mode(TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP, *mode);
		}
	}

	return ret;
}

static void lpm_enter_partial_io_mode(void)
{
	u32 reg = 0;
	u32 timeout = TIMEOUT_10MS;

	/* Unlock wkup_ctrl_mmr region 6 */
	ctrlmmr_unlock(WKUP_CTRL_BASE, 6);

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

static void lpm_get_wake_reason_params(struct tisci_msg_lpm_wake_reason_resp *wake_params)
{
	u8 val = (u8) readl(WKUP_CTRL_BASE + WKUP_PIN_SRC_REG);

	/* Get the wake parameters for other modes */
	lpm_get_wake_info(wake_params);

	/* If no valid mode is entered, check if partial I/O was the last entered mode */
	if (wake_params->mode == TISCI_MSG_VALUE_SLEEP_MODE_INVALID) {
		/* If pin number is valid, update the correct mode, source and pin */
		if ((val >= CAN_IO_WKUP_PIN_NUM_START) && (val <= CAN_IO_WKUP_PIN_NUM_END)) {
			wake_params->wake_pin = val;
			wake_params->wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_CAN_IO;
			wake_params->mode = TISCI_MSG_VALUE_SLEEP_MODE_PARTIAL_IO;
		} else if (val == TISCI_MSG_VALUE_LPM_WAKE_PIN_INVALID) {
			/* If this is partial I/O exit but the wakeup source is unknown */
			wake_params->wake_pin = val;
			wake_params->wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_INVALID;
			wake_params->mode = TISCI_MSG_VALUE_SLEEP_MODE_PARTIAL_IO;
		} else {
			/* Empty else condition to fix MISRA.IF.NO_ELSE violation. */
		}
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
	u8 mode = TISCI_MSG_VALUE_SLEEP_MODE_INVALID;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_PREPARE_SLEEP);

	/* Select low power mode if mode is not yet locked and requested mode is "DM managed" mode */
	if ((lpm_locked == SFALSE) && (req->mode == TISCI_MSG_VALUE_SLEEP_MODE_DM_MANAGED)) {
		if (lpm_select_sleep_mode(&mode) == SUCCESS) {
			lpm_locked = STRUE;
			req->mode = mode;
		} else {
			ret = -EFAIL;
		}
	} else if (lpm_locked == STRUE) {
		/* If lock is already applied, return failure */
		ret = -EFAIL;
	} else {
		/* For all other modes, skip mode selection and save padconfig values */
		lpm_locked = STRUE;
		mode = req->mode;

		/* Scan and save the padconfig values */
		if (ret == SUCCESS) {
			ret = lpm_sleep_save_main_padconf(NULL);
		}

		if ((mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) && (ret == SUCCESS)) {
			ret = lpm_sleep_save_mcu_padconf(NULL);
		}
	}

	/* Reset the wake reason values */
	lpm_reset_wake_reason_params();
	lpm_reset_partial_io_wake_info();

	switch (mode) {
	case TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR:
	case TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP:
	case TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY:
		if (mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
			/* Return failure if the device does not support IO only plus DDR mode */
			if ((ret == SUCCESS) && (mode != DEEPEST_LOW_POWER_MODE)) {
				ret = -EFAIL;
			}
		}

		if (ret == SUCCESS) {
			/* Parse and store the mode info and ctx address in the prepare sleep message */
			lpm_populate_prepare_sleep_data(req);

			/*
			 * Clearing all wakeup interrupts from VIM. Even if we are cleaning interrupts
			 * from VIM, if the wakeup interrupt is still active it will be able to wake
			 * the soc from LPM. This will only clear any unwanted pending wakeup interrupts
			 */
			lpm_clear_all_wakeup_interrupt();
		}
		break;
	case TISCI_MSG_VALUE_SLEEP_MODE_PARTIAL_IO:
		if (ret == SUCCESS) {
			/* Suspend DM so that in case of failure, idle hook is not executed */
			ret = lpm_sleep_suspend_dm();
		}

		if (ret == SUCCESS) {
			/*
			 * Wait for tifs to reach WFI in both the failed and successful case.
			 * but update the ret value only if it was SUCCESS previously
			 */
			ret = lpm_sleep_wait_for_tifs_wfi();
		}

		if (ret == SUCCESS) {
			/* Enable CANUART IO daisy chain and enter partial io mode */
			lpm_enter_partial_io_mode();
		} else {
			lpm_hang_abort();
		}
		break;
	default: ret = -EFAIL;
		break;
	}

	return ret;
}

s32 dm_enter_sleep_handler(u32 *msg_recv)
{
	struct tisci_msg_enter_sleep_req *req =
		(struct tisci_msg_enter_sleep_req *) msg_recv;

	s32 ret = SUCCESS;
	u8 mode = req->mode;
	u32 temp_sleep_status = 0;

	enter_sleep_status = 0;

	/* Check if the input mode is valid */
	if (mode > TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
		ret = -EINVAL;
	}

	/*
	 * Wait for tifs to reach WFI in both the failed and successful case.
	 * but update the ret value only if it was SUCCESS previously
	 */
	if (ret == SUCCESS) {
		ret = lpm_sleep_wait_for_tifs_wfi();
	} else {
		lpm_sleep_wait_for_tifs_wfi();
	}

	/*
	 * Since, once power master reaches WFI, power master is only recoverable
	 * by reseting the  power master. Only update the ret value only if it was
	 * SUCCESS previously
	 */
	if (ret == SUCCESS) {
		ret = lpm_suspend_power_master();
		enter_sleep_status |= LPM_SUSPEND_POWERMASTER;
	} else {
		lpm_suspend_power_master();
		enter_sleep_status |= LPM_SUSPEND_POWERMASTER;
	}

	if (ret == SUCCESS) {
		ret = osal_suspend_dm_application();
		enter_sleep_status |= LPM_SUSPEND_DM_APPLICATION;
	}

	if (ret == SUCCESS) {
		ret = devices_deinit_flags();
		enter_sleep_status |= LPM_DEVICE_DEINIT;
	} else {
		devices_deinit_flags();
		enter_sleep_status |= LPM_DEVICE_DEINIT;
	}

	if (ret == SUCCESS) {
		/* As PADCFGs have already been saved, set the flag */
		enter_sleep_status |= LPM_SAVE_MAIN_PADCONFIG;

		/* MCU PADCFGs values is lost only during IO DDR mode */
		if (mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
			enter_sleep_status |= LPM_SAVE_MCU_PADCONFIG;
		}
	}

	if (ret == SUCCESS) {
		ret = lpm_save_mmr_lock();
		enter_sleep_status |= LPM_SAVE_MMR_LOCK;
	}

	if (ret == SUCCESS) {
		ret = lpm_sleep_suspend_gtc();
		enter_sleep_status |= LPM_SUSPEND_GTC;
	}

	if (ret == SUCCESS) {
		ret = clks_suspend();
		enter_sleep_status |= LPM_CLOCK_SUSPEND;
	}

	if ((mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) && (ret == SUCCESS)) {
		ret = lpm_suspend_RM_context();
		enter_sleep_status |= LPM_RM_DEINIT;
	}

	if (ret == SUCCESS) {
		ret = lpm_sleep_suspend_dm();
		enter_sleep_status |= LPM_SUSPEND_DM;
	}

	if ((mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) && (ret == SUCCESS)) {
		ret = lpm_sleep_save_wkup_periph_config();
		enter_sleep_status |= LPM_SAVE_WKUP_PERIPH_CFG;
	}

	if (ret == SUCCESS) {
		ret = lpm_sleep_jump_to_dm_Stub();
	}

	/* If there is any failure, inform TIFS using abort message */
	if (ret != SUCCESS) {
		if (lpm_resume_send_enter_sleep_abort_message() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	temp_sleep_status = enter_sleep_status;

	if ((temp_sleep_status & LPM_SUSPEND_GTC) == LPM_SUSPEND_GTC) {
		if (lpm_resume_gtc() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SAVE_MCU_PADCONFIG) == LPM_SAVE_MCU_PADCONFIG) {
		if (lpm_resume_restore_mcu_padconf() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SAVE_WKUP_PERIPH_CFG) == LPM_SAVE_WKUP_PERIPH_CFG) {
		if (lpm_resume_restore_wkup_periph() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((ret == SUCCESS) && (mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR)) {
		if (lpm_copy_fs_stub() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SUSPEND_DM) == LPM_SUSPEND_DM) {
		if (lpm_resume_dm() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SAVE_MMR_LOCK) == LPM_SAVE_MMR_LOCK) {
		if (lpm_restore_mmr_lock() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_CLOCK_SUSPEND) == LPM_CLOCK_SUSPEND) {
		if (clks_resume() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((ret == SUCCESS) || ((temp_sleep_status & LPM_RM_DEINIT) == LPM_RM_DEINIT)) {
		if (lpm_resume_restore_RM_context() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SUSPEND_POWERMASTER) == LPM_SUSPEND_POWERMASTER) {
		if (lpm_resume_send_core_resume_message() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_DEVICE_DEINIT) == LPM_DEVICE_DEINIT) {
		if (devices_init() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SUSPEND_DM_APPLICATION) == LPM_SUSPEND_DM_APPLICATION) {
		if (osal_resume_dm_application() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SAVE_MAIN_PADCONFIG) == LPM_SAVE_MAIN_PADCONFIG) {
		if (lpm_resume_restore_main_padconf() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	if ((temp_sleep_status & LPM_SUSPEND_POWERMASTER) == LPM_SUSPEND_POWERMASTER) {
		if (lpm_resume_release_reset_of_power_master() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	/* Remove isolation from CANUART pins */
	if (ret == SUCCESS) {
		if (unload_magic_words_thru_wkup_mmr() != SUCCESS) {
			lpm_hang_abort();
		}
	}

	/* Enable interrupts used by DM firmware */
	osal_dm_enable_interrupt();

	/* Open the mode selection lock and clear constraints */
	lpm_clear_sleep_data();

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
	resp->wake_timestamp = 0;

	/* Update the wake reason params */
	lpm_get_wake_reason_params(resp);

	return ret;
}

s32 dm_set_io_isolation_handler(u32 *msg_recv)
{
	struct tisci_msg_set_io_isolation_req *req =
		(struct tisci_msg_set_io_isolation_req *) msg_recv;
	s32 ret = EFAIL;
	u32 reg;
	u32 i = 0;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_SET_IO_ISOLATION);

	/* Unlock partition 6 of wakeup ctrl mmr */
	ctrlmmr_unlock(WKUP_CTRL_BASE, 6);
	if (req->state == TISCI_MSG_VALUE_IO_ENABLE) {
		writel(WKUP_CTRL_DEEPSLEEP_CTRL_ENABLE_IO, (WKUP_CTRL_BASE + WKUP_CTRL_DEEPSLEEP_CTRL));
		writel(WKUP_CTRL_PMCTRL_IO_GLB_ENABLE_IO, (WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_GLB));

		/* Set global isobypass (valid for MCU IOs only) */
		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_ISO_BYPASS;
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);

		/* Set global wuen */
		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN;
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);

		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN;
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);

		/* Set global isoin */
		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL;
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);

		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL;
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);

		/* Wait for wu clock state to be 1*/
		i = 0;
		do {
			ret = ETIMEDOUT;
			reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
			if ((reg & WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) == WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) {
				ret = SUCCESS;
				break;
			}
			osal_delay(1);
		} while (i++ < TIMEOUT_10MS);
		i = 0;
		do {
			ret = ETIMEDOUT;
			reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);
			if ((reg & WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) == WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) {
				ret = SUCCESS;
				break;
			}
			osal_delay(1);
		} while (i++ < TIMEOUT_10MS);
	} else if (req->state == TISCI_MSG_VALUE_IO_DISABLE) {
		/* Clear global wuen */
		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN);
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);

		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN);
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);

		/* Clear global isoin */
		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL);
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_0);

		reg = readl(WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL);
		writel(reg, WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_1);

		writel(WKUP_CTRL_DEEPSLEEP_CTRL_DISABLE_IO, (WKUP_CTRL_BASE + WKUP_CTRL_DEEPSLEEP_CTRL));
		writel(WKUP_CTRL_PMCTRL_IO_GLB_DISABLE_IO, (WKUP_CTRL_BASE + WKUP_CTRL_PMCTRL_IO_GLB));

		ret = SUCCESS;
	} else {
		/* Do Nothing */
	}
	return ret;
}

s32 dm_lpm_set_device_constraint(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	struct tisci_msg_lpm_set_device_constraint_req *req =
		(struct tisci_msg_lpm_set_device_constraint_req *) msg_recv;
	struct tisci_msg_lpm_set_device_constraint_resp *resp =
		(struct tisci_msg_lpm_set_device_constraint_resp *) msg_recv;
	struct device *dev = NULL;
	u32 id = req->id;
	u8 state = req->state;
	u8 host_id = req->hdr.host;
	u8 host_idx;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_SET_DEVICE_CONSTRAINT);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID, id);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_VAL, state);

	resp->hdr.flags = 0U;

	if (state == TISCI_MSG_VALUE_STATE_SET) {
		/* Prepare the host and device for setting constraints - Exclusive rights valued */
		ret = device_prepare_exclusive(host_id, id, &host_idx, &dev);
		if (ret == SUCCESS) {
			/* Set constraint */
			dev_cons[id] |= (1U << host_idx);
		}
	} else {
		/* Prepare the host and device for clearing constraints - Can be cleared irrespective of exclusive rights */
		ret = device_prepare_nonexclusive(host_id, id, &host_idx, &dev);
		if (ret == SUCCESS) {
			/* Clear constraint */
			dev_cons[id] &= ~(1U << host_idx);
		}
	}

	return ret;
}

s32 dm_lpm_get_device_constraint(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	struct tisci_msg_lpm_get_device_constraint_req *req =
		(struct tisci_msg_lpm_get_device_constraint_req *) msg_recv;
	struct tisci_msg_lpm_get_device_constraint_resp *resp =
		(struct tisci_msg_lpm_get_device_constraint_resp *) msg_recv;
	struct device *dev = NULL;
	u32 id = req->id;
	u8 host_id = req->hdr.host;
	u8 host_idx;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_GET_DEVICE_CONSTRAINT);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID, id);

	resp->hdr.flags = 0U;

	/* Prepare the host and device for getting constraint value */
	ret = device_prepare_nonexclusive(host_id, id, &host_idx, &dev);
	if (ret == SUCCESS) {
		/* Get constraint value on this device put by requesting host */
		resp->state = (dev_cons[id] & (1U << host_idx)) >> host_idx;
	}

	return ret;
}

s32 dm_lpm_set_latency_constraint(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	struct tisci_msg_lpm_set_latency_constraint_req *req =
		(struct tisci_msg_lpm_set_latency_constraint_req *) msg_recv;
	struct tisci_msg_lpm_set_latency_constraint_resp *resp =
		(struct tisci_msg_lpm_set_latency_constraint_resp *) msg_recv;
	u16 resume_latency = req->resume_latency;
	u8 state = req->state;
	u8 host_id = req->hdr.host;
	u8 host_idx;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_SET_LATENCY_CONSTRAINT);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_LATENCY, resume_latency);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_VAL, state);

	resp->hdr.flags = 0U;

	/* Check if current host is valid and get lookup host ID */
	host_idx = host_idx_lookup(host_id);
	if ((host_idx == HOST_IDX_NONE) || (host_idx >= HOST_ID_CNT)) {
		ret = -EFAIL;
	}

	if (ret == SUCCESS) {
		if (state == TISCI_MSG_VALUE_STATE_SET) {
			/* If latency constraint has already been set, take the minimum */
			if ((latency[host_idx] & LPM_RESUME_LATENCY_VALID_FLAG) == LPM_RESUME_LATENCY_VALID_FLAG) {
				resume_latency = MIN(resume_latency, (u16) (latency[host_idx]));
			}
			/* Set latency constraint */
			latency[host_idx] = (LPM_RESUME_LATENCY_VALID_FLAG | resume_latency);
		} else {
			/* Clear latency constraint */
			latency[host_idx] = 0U;
		}
	}

	return ret;
}

s32 dm_lpm_get_latency_constraint(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	struct tisci_msg_lpm_get_latency_constraint_req *req =
		(struct tisci_msg_lpm_get_latency_constraint_req *) msg_recv;
	struct tisci_msg_lpm_get_latency_constraint_resp *resp =
		(struct tisci_msg_lpm_get_latency_constraint_resp *) msg_recv;
	u8 host_id = req->hdr.host;
	u8 host_idx;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_GET_LATENCY_CONSTRAINT);

	resp->hdr.flags = 0U;

	/* Check if current host is valid and get lookup host ID */
	host_idx = host_idx_lookup(host_id);
	if ((host_idx == HOST_IDX_NONE) || (host_idx >= HOST_ID_CNT)) {
		ret = -EFAIL;
	}

	if (ret == SUCCESS) {
		/* Get latency constraint value for given host */
		resp->state = ((latency[host_idx] & LPM_RESUME_LATENCY_VALID_FLAG) == LPM_RESUME_LATENCY_VALID_FLAG);
		resp->resume_latency = (u16) latency[host_idx];
	}

	return ret;
}

s32 dm_lpm_get_next_sys_mode(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	struct tisci_msg_lpm_get_next_sys_mode_resp *resp =
		(struct tisci_msg_lpm_get_next_sys_mode_resp *) msg_recv;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_GET_NEXT_SYS_MODE);

	resp->hdr.flags = 0U;

	if (lpm_locked == STRUE) {
		resp->mode = lpm_get_selected_sleep_mode();
	} else {
		resp->mode = TISCI_MSG_VALUE_SLEEP_MODE_NOT_SELECTED;
	}

	return ret;
}

s32 dm_lpm_get_next_host_state(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	u8 mode;
	u8 state;
	struct tisci_msg_lpm_get_next_host_state_req *req =
		(struct tisci_msg_lpm_get_next_host_state_req *) msg_recv;
	struct tisci_msg_lpm_get_next_host_state_resp *resp =
		(struct tisci_msg_lpm_get_next_host_state_resp *) msg_recv;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_GET_NEXT_HOST_STATE);

	resp->hdr.flags = 0U;

	if (lpm_locked == STRUE) {
		mode = lpm_get_selected_sleep_mode();

		if (mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY) {
			/* MCU host needs to stay ON during MCU only low power mode */
			if (req->hdr.host == MCU_HOST_ID) {
				state = TISCI_MSG_VALUE_HOST_STATE_ON;
			} else {
				state = TISCI_MSG_VALUE_HOST_STATE_OFF;
			}
		} else {
			state = TISCI_MSG_VALUE_HOST_STATE_OFF;
		}
	} else {
		/* Return invalid state */
		state = TISCI_MSG_VALUE_HOST_STATE_INVALID;
	}

	resp->state = state;

	return ret;
}

s32 dm_lpm_abort(u32 *msg_recv)
{
	s32 ret = SUCCESS;
	struct tisci_msg_lpm_abort_resp *resp =
		(struct tisci_msg_lpm_abort_resp *) msg_recv;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_LPM_ABORT);

	resp->hdr.flags = 0U;

	if (lpm_locked == STRUE) {
		lpm_clear_sleep_data();
	}

	return ret;
}
