/*
 * DM Stub Firmware
 *
 * DM Stub main low power functionality
 *
 * Copyright (C) 2021-2025, Texas Instruments Incorporated
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

#include <lib/bitops.h>
#include <tisci/lpm/tisci_lpm.h>
#include <types/errno.h>
#include <lpscs.h>
#include <pll.h>
#include <mcu_ctrl_mmr.h>
#include <wkup_ctrl_mmr.h>
#include <wake_data.h>
#include <lpm_data.h>

#include "lpm_io.h"
#include "ctrlmmr_raw.h"
#include "ddr.h"
#include "lpm_serial_8250.h"
#include "lpm_string.h"
#include "lpm_trace.h"
#include "sec_proxy.h"
#include "timeout.h"
#include "vim_raw.h"
#include "lpm_string.h"
#include "string.h"
#include "cache.h"

extern s32 __attribute__((noinline)) dm_stub_entry(void);
extern s32 __attribute__((noinline)) dm_stub_resume(void);
extern void dm_stub_irq_handler(void);
extern void lpm_get_wake_info(struct tisci_msg_lpm_wake_reason_resp *wkup_params);
extern void lpm_populate_prepare_sleep_data(struct tisci_msg_prepare_sleep_req *p);
extern void lpm_clear_all_wakeup_interrupt(void);
extern u8 lpm_get_wkup_pin_number_padconf(u32 wkup_src);
extern u8 lpm_get_selected_sleep_mode(void);
extern void lpm_reset_wake_reason_params(void);
s32 unload_magic_words_thru_wkup_mmr(void);

#define TCM_SIZE            0x8000U
#define TCMB_BASE_ADDR      0U

static void enter_WFI(void)
{
	__asm volatile ("\tWFI");
}
static void enable_intr(void)
{
	__asm volatile ("\tCPSIE I");
}

/* Variable to store the last wakeup information */
static struct tisci_msg_lpm_wake_reason_resp g_wkup_params = {
	.mode		= TISCI_MSG_VALUE_SLEEP_MODE_INVALID,
	.wake_pin	= TISCI_MSG_VALUE_LPM_WAKE_PIN_INVALID,
	.wake_source	= TISCI_MSG_VALUE_LPM_WAKE_SOURCE_INVALID,
};

static struct tisci_msg_prepare_sleep_req g_params;

extern u8 dm_stub_arr[TCM_SIZE];

/*
 * Define a setup time of 4x default for MOSC
 *
 * This allows for better clock stability once R5 begins
 * execution again.
 */
#define PMCTRL_MOSC_SETUP_TIME          0xFC000U

void lpm_clear_all_wakeup_interrupt(void)
{
	u32 i;

	for (i = 0; i < (u32) WAKEUP_SOURCE_MAX; i++) {
		vim_clear_intr(soc_wake_sources_data[i].int_num);
	}
}

static void lpm_abort(void)
{
	volatile int a = 0x1234;

	while (a != 0) {
	}
}

#ifdef CONFIG_LPM_32_BIT_DDR

static s32 enter_ddr_low_power_mode(void)
{
	s32 ret = 0;

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
		/* Configure DDR for low power mode entry */
		ret = ddr_enter_io_ddr_mode();
	} else {
		ret = ddr_enter_low_power_mode();

		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_DATA_ISO,
					       MDCTL_STATE_DISABLE, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
		}

		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_LOCAL,
					       MDCTL_STATE_DISABLE, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
		}

		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_CFG_ISO,
					       MDCTL_STATE_DISABLE, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
		}
	}

	return ret;
}

static s32 exit_ddr_low_power_mode(void)
{
	s32 ret = 0;

	psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_LOCAL,
			       MDCTL_STATE_ENABLE, 0);
	psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

	ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);

	if (ret == 0) {
		psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_CFG_ISO,
				       MDCTL_STATE_ENABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
	}

	if (ret == 0) {
		psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_DATA_ISO,
				       MDCTL_STATE_ENABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
	}

	if (ret == 0) {
		ret = ddr_exit_low_power_mode();
	}

	return ret;
}

static s32 load_magic_words_through_mmr(void)
{
	u32 timeout = TIMEOUT_10_MS;
	s32 ret = SUCCESS;

	/* Program the OFF mode MMR in case of IO Only plus DDR mode. */
	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
		writel(WKUP_CANUART_OFF_MAGIC_WORD, WKUP_CTRL_MMR_BASE + CANUART_WAKE_OFF_MODE);
	}

	/* Program the CAN IO MMR. */
	writel(0x0U, WKUP_CTRL_MMR_BASE + CANUART_WAKE_CTRL);
	writel((WKUP_CANUART_MAGIC_WRD | WKUP_CANUART_MAGIC_WRD_LD_EN), WKUP_CTRL_MMR_BASE + CANUART_WAKE_CTRL);

	/* Wait for CAN_ONLY_IO signal to be 1 */
	while ((timeout > 0U) && ((readl(WKUP_CTRL_MMR_BASE + CANUART_WAKE_STAT1)) != WKUP_CANUART_CAN_IO_ISO_SET)) {
		--timeout;
	}
	if (timeout == 0U) {
		ret = -EFAIL;
	}

	/* Clear the magic word to prevent any other word loading */
	writel((~WKUP_CANUART_MAGIC_WRD) | WKUP_CANUART_MAGIC_WRD_LD_DIS, WKUP_CTRL_MMR_BASE + CANUART_WAKE_CTRL);

	return ret;
}

s32 unload_magic_words_thru_wkup_mmr(void)
{
	u32 timeout = TIMEOUT_10_MS;
	s32 ret = SUCCESS;

	/* Unload magic words - Transition load enable bit from 1 to 0 with magic word written. */
	writel((WKUP_CANUART_MAGIC_WRD | WKUP_CANUART_MAGIC_WRD_LD_EN), WKUP_CTRL_MMR_BASE + CANUART_WAKE_CTRL);

	/* Unload magic words - Clear CAN IO magic word load enable bit with magic word written. */
	writel((WKUP_CANUART_MAGIC_WRD | WKUP_CANUART_MAGIC_WRD_LD_DIS), WKUP_CTRL_MMR_BASE + CANUART_WAKE_CTRL);

	/* Wait for CAN ONLY IO signal to be 0 */
	while ((timeout > 0U) && (((readl(WKUP_CTRL_MMR_BASE + CANUART_WAKE_STAT1) == WKUP_CANUART_CAN_IO_ISO_CLRD)) == SFALSE)) {
		--timeout;
	}
	if (timeout == 0U) {
		ret = -EFAIL;
	}

	/* Reset the OFF mode MMRs and CAN IO mode MMRs. */
	writel(0x0, WKUP_CTRL_MMR_BASE + CANUART_WAKE_OFF_MODE);
	writel(WKUP_CANUART_MAGIC_WRD_LD_DIS, WKUP_CTRL_MMR_BASE + CANUART_WAKE_CTRL);

	return ret;
}

#else
static s32 enter_ddr_low_power_mode(void)
{
	s32 ret = 0;

	psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_DATA_ISO,
			       MDCTL_STATE_DISABLE, 0);
	psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

	ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);

	if (ret == 0) {
		/* Configure DDR for low power mode entry */
		ret = ddr_enter_low_power_mode();
	}

	if (ret == 0) {
		psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_LOCAL,
				       MDCTL_STATE_DISABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
	}

	if (ret == 0) {
		ddr_enable_retention();

		psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_CFG_ISO,
				       MDCTL_STATE_DISABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);

		/* Reset isolate DDR */
		writel(DS_RESET_MASK, WKUP_CTRL_MMR_BASE + DS_DDR0_RESET);
	}

	return ret;
}

static s32 exit_ddr_low_power_mode(void)
{
	s32 ret = 0;

	ddr_disable_retention();

	psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_LOCAL,
			       MDCTL_STATE_ENABLE, 0);
	psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

	ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);

	if (ret == 0) {
		psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_CFG_ISO,
				       MDCTL_STATE_ENABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);

		/* Remove DDR Reset isolation */
		writel(DS_RESET_UNMASK, WKUP_CTRL_MMR_BASE + DS_DDR0_RESET);
	}

	if (ret == 0) {
		ret = ddr_exit_low_power_mode();
	}

	if (ret == 0) {
		psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_EMIF_DATA_ISO,
				       MDCTL_STATE_ENABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, DDR_PD);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, DDR_PD);
	}

	if (ret == 0) {
		ret = ddr_deepsleep_exit_training();
	}

	return ret;
}

static s32 load_magic_words_through_mmr(void)
{
	return SUCCESS;
}

s32 unload_magic_words_thru_wkup_mmr(void)
{
	return SUCCESS;
}
#endif

static void enter_io_only_ddr(void)
{
	/* Ensure that PMIC EN control from SOC is selected */
	writel((WKUP0_PMCTRL_SYS_LPM_EN_PMIC | WKUP0_LPM_PMIC_OUT_EN), (WKUP_CTRL_MMR_BASE + PMCTRL_SYS));

	/* Enter IO DDR mode */
	writel((WKUP0_PMCTRL_SYS_LPM_EN_PMIC | WKUP0_LPM_PMIC_OUT_DIS), WKUP_CTRL_MMR_BASE + PMCTRL_SYS);

	/* If PMIC fails to suspend the system, hang abort */
	lpm_abort();
}

static void clock_gate_legacy_peripherals(sbool enable)
{
	u32 timeout = TIMEOUT_10_MS;

	if (enable) {
		writel(WKUP_EN_CLKSTOP_ALL, WKUP_CTRL_MMR_BASE + DM_CLKSTOP_EN);
		writel(WKUP_EN_GRP_CLKSTOP_REQ, WKUP_CTRL_MMR_BASE + DM_GRP_CLKSTOP_REQ);
		while ((timeout > 0U) && (readl(WKUP_CTRL_MMR_BASE + DM_GRP_CLKSTOP_ACK) != WKUP_EN_GRP_CLKSTOP_ACK)) {
			--timeout;
		}
		if (timeout == 0U) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_CLK_GATE);
		}
	} else {
		writel(WKUP_DIS_GRP_CLKSTOP_REQ, WKUP_CTRL_MMR_BASE + DM_GRP_CLKSTOP_REQ);
		while ((timeout > 0U) && (readl(WKUP_CTRL_MMR_BASE + DM_GRP_CLKSTOP_ACK) != WKUP_DIS_GRP_CLKSTOP_ACK)) {
			--timeout;
		}
		if (timeout == 0U) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_CLK_GATE);
		}
		writel(0, WKUP_CTRL_MMR_BASE + DM_CLKSTOP_EN);
	}
}

static s32 set_usb_reset_isolation(void)
{
	s32 ret = 0;
	u32 i = 0;

	/* Save the state of USB LPSCs */
	for (i = 0; i < num_usb_lpscs; i++) {
		/* Get the current state of USBx LPSC */
		usb_lpscs[i].state = psc_raw_lpsc_get_state(MAIN_PSC_BASE, usb_lpscs[i].lpsc);

		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, usb_lpscs[i].iso_lpsc,
					       MDCTL_STATE_DISABLE, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, usb_lpscs[i].pd);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, usb_lpscs[i].pd);
		}

		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, usb_lpscs[i].lpsc,
					       MDCTL_STATE_DISABLE, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, usb_lpscs[i].pd);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, usb_lpscs[i].pd);
		}
	}

	if (ret == 0) {
		/* Reset isolate USB */
		writel(DS_RESET_MASK, WKUP_CTRL_MMR_BASE + DS_USB0_RESET);
		writel(DS_RESET_MASK, WKUP_CTRL_MMR_BASE + DS_USB1_RESET);

		/* Disable source of USB clock */
		pll_hsdiv_ctrl(usb_pll.base, usb_pll.hsdiv[0], PLL_DISABLE);
	}

	return ret;
}

static s32 release_usb_reset_isolation(void)
{
	s32 ret = 0;
	u32 i = 0;

	/* Enable source of USB clock */
	pll_hsdiv_ctrl(usb_pll.base, usb_pll.hsdiv[0], PLL_ENABLE);

	/* Restore the state of USB LPSCs */
	for (i = 0; i < num_usb_lpscs; i++) {
		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, usb_lpscs[i].lpsc,
					       usb_lpscs[i].state, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, usb_lpscs[i].pd);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, usb_lpscs[i].pd);
		}

		if (ret == 0) {
			psc_raw_lpsc_set_state(MAIN_PSC_BASE, usb_lpscs[i].iso_lpsc,
					       usb_lpscs[i].state, 0);
			psc_raw_pd_initiate(MAIN_PSC_BASE, usb_lpscs[i].pd);

			ret = psc_raw_pd_wait(MAIN_PSC_BASE, usb_lpscs[i].pd);
		}
	}

	/* Remove reset isolation of USB */
	if (ret == 0) {
		writel(DS_RESET_UNMASK, WKUP_CTRL_MMR_BASE + DS_USB0_RESET);
		writel(DS_RESET_UNMASK, WKUP_CTRL_MMR_BASE + DS_USB1_RESET);
	}

	return ret;
}

static s32 disable_main_lpsc(const struct pd_lpsc *lpscs, u32 n_lpscs)
{
	u32 i;
	s32 ret = 0;

	for (i = 0; i < n_lpscs; i++) {
		psc_raw_lpsc_set_state(MAIN_PSC_BASE, lpscs[i].lpsc,
				       MDCTL_STATE_DISABLE, 0);
		psc_raw_pd_initiate(MAIN_PSC_BASE, lpscs[i].pd);

		ret = psc_raw_pd_wait(MAIN_PSC_BASE, lpscs[i].pd);
		if (ret != 0) {
			break;
		}
	}

	return ret;
}

static void bypass_main_pll(void)
{
	u32 i;

	/* Save the PLL config */
	for (i = 0; i < num_main_plls_save_rstr; i++) {
		pll_save(main_plls_save_rstr[i]);
	}

	/* Bypass and disable all PLL in MAIN_PLLCTRL,
	 * except clock for Debug, PLL0, PLL15
	 */
	for (i = 0; i < num_main_plls_dis; i++) {
		pll_disable(main_plls_dis[i]);
	}
}

static void wait_for_debug(void)
{
	while (g_params.debug_flags != 0U) {
	}
}

static void config_wake_sources(void)
{
	int i;
	u32 val = 0;

	for (i = 0; i < WAKEUP_SOURCE_MAX; i++) {
		/* Enable interrupt */
		vim_set_intr_enable(soc_wake_sources_data[i].int_num,
				    INTR_ENABLE);

		/* Set wkup enable bit in MMR */
		val |= BIT(soc_wake_sources_data[i].wkup_idx);
	}
	/* Write all bits to enable at once */
	writel(val, (WKUP_CTRL_MMR_BASE + WKUP0_EN));

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY) {
		/* Enable MCU_IPC interrupt */
		vim_set_intr_enable(MCU_IPC_INTERRUPT_NUMBER, INTR_ENABLE);
	}
}

static void disable_wake_sources(void)
{
	u32 i;

	/* Disable all wake up interrupts */
	for (i = 0; i < (u32) WAKEUP_SOURCE_MAX; i++) {
		vim_set_intr_enable(soc_wake_sources_data[i].int_num, INTR_DISABLE);
	}
	/* Clear all bits in WKUP0_EN */
	writel(0, (WKUP_CTRL_MMR_BASE + WKUP0_EN));

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY) {
		/* Disable MCU_IPC interrupt */
		vim_set_intr_enable(MCU_IPC_INTERRUPT_NUMBER, INTR_DISABLE);
	}
}

static void lpm_update_wkup_pin_and_mode(void)
{
	g_wkup_params.wake_pin = lpm_get_wkup_pin_number_padconf(g_wkup_params.wake_source);
	g_wkup_params.mode = g_params.mode;
}

static void config_gpio_clk_mux(u8 clk_src)
{
	writel(clk_src, (MCU_CTRL_MMR_BASE + MCU_CTRL_MMR_CFG0_MCU_GPIO_CLKSEL));
}

static void enable_gpio_wake_up(void)
{
	writel(MCU_CTRL_MMR_CFG0_MCU_GPIO_WKUP_CTRL_ENABLE, (MCU_CTRL_MMR_BASE + MCU_CTRL_MMR_CFG0_MCU_GPIO_WKUP_CTRL));
}

static void disable_gpio_wake_up(void)
{
	writel(MCU_CTRL_MMR_CFG0_MCU_GPIO_WKUP_CTRL_DISABLE, (MCU_CTRL_MMR_BASE + MCU_CTRL_MMR_CFG0_MCU_GPIO_WKUP_CTRL));
}

static int enable_main_remain_pll(void)
{
	u32 i;
	s32 ret = 0;

	for (i = 0; i < num_main_plls_save_rstr; i++) {
		if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) && (main_plls_save_rstr[i] == &main_pll12)) {
			/* Skip restoring PLL for DDR */
			continue;
		}
		ret = pll_restore(main_plls_save_rstr[i]);
		if (ret != 0) {
			break;
		}
	}

	return ret;
}

static s32 disable_mcu_domain(void)
{
	u32 i;
	s32 ret = 0;

	for (i = 0; i < num_mcu_lpscs; i++) {
		psc_raw_lpsc_set_state(MCU_PSC_BASE, mcu_lpscs[i].lpsc,
				       MDCTL_STATE_DISABLE, 0);
		psc_raw_pd_initiate(MCU_PSC_BASE, mcu_lpscs[i].pd);

		ret = psc_raw_pd_wait(MCU_PSC_BASE, mcu_lpscs[i].pd);
		if (ret != 0) {
			break;
		}
	}

	for (i = 0; i < num_mcu_pds; i++) {
		if (ret == 0) {
			psc_raw_pd_set_state(MCU_PSC_BASE, mcu_pds[i],
					     PDCTL_STATE_OFF, 0);
			psc_raw_pd_initiate(MCU_PSC_BASE, mcu_pds[i]);

			ret = psc_raw_pd_wait(MCU_PSC_BASE, mcu_pds[i]);
		}
	}

	return ret;
}

static s32 enable_mcu_lpsc(void)
{
	u32 i;
	s32 ret = 0;

	for (i = 0; i < num_mcu_lpscs; i++) {
		if (ret == 0) {
			psc_raw_lpsc_set_state(MCU_PSC_BASE, mcu_lpscs[i].lpsc,
					       MDCTL_STATE_ENABLE, 0);
			psc_raw_pd_initiate(MCU_PSC_BASE, mcu_lpscs[i].pd);

			ret = psc_raw_pd_wait(MCU_PSC_BASE, mcu_lpscs[i].pd);
		}
	}

	return ret;
}

static s32 enable_dm_lpsc(void)
{
	s32 ret = 0;

	psc_raw_lpsc_set_state(MAIN_PSC_BASE, LPSC_MAIN_DM,
			       MDCTL_STATE_ENABLE, 0);
	psc_raw_pd_initiate(MAIN_PSC_BASE, PD_GP_CORE_CTL);
	ret = psc_raw_pd_wait(MAIN_PSC_BASE, PD_GP_CORE_CTL);

	return ret;
}

/* Send TISCI ROM Boot image message containing location
 * and boot address to load FS stub from SPS Memory
 */

#ifndef CONFIG_LPM_DM_STUB_NO_FS_STUB
static s32 send_tisci_msg_firmware_load(void)
{
	s32 ret = 0;

	struct tisci_msg_firmware_load_resp resp;
	struct tisci_msg_firmware_load_req req;

	req.hdr.type = TISCI_MSG_FIRMWARE_LOAD;
	req.hdr.flags = (8U << 24U);
	req.image_addr = CONFIG_TIFSFW_SPS_BASE;
	req.image_size = CONFIG_TIFSFW_SPS_LEN;

	ret = sproxy_send_msg_rom(&req, sizeof(req));
	if (ret == 0) {
		lpm_memset(&resp, 0, sizeof(resp));

		ret = sproxy_receive_msg_rom(&resp, sizeof(resp));
		if (ret == 0) {
			if ((resp.hdr.type != MSG_FIRMWARE_LOAD_RESULT) ||
			    (resp.hdr.flags != MSG_FLAG_CERT_AUTH_PASS)) {
				ret = -EINVAL;
			}
		}
	}

	return ret;
}

static s32 receive_tisci_msg_continue_resume_req(void)
{
	struct tisci_msg_continue_resume_req req;
	s32 ret = 0;

	lpm_memset(&req, 0, sizeof(req));

	ret = sproxy_receive_msg_rom(&req, sizeof(req));
	if (ret == 0) {
		if (req.hdr.type != TISCI_MSG_CONTINUE_RESUME) {
			ret = -EINVAL;
		}
	}

	return ret;
}

static s32 send_tisci_msg_continue_resume_resp(void)
{
	s32 ret = 0;
	struct tisci_msg_continue_resume_resp resp;

	resp.hdr.type = TISCI_MSG_CONTINUE_RESUME;
	resp.hdr.flags = TISCI_MSG_FLAG_ACK;

	resp.ctx_lo = g_params.ctx_lo;
	resp.ctx_hi = g_params.ctx_hi;

	ret = sproxy_send_msg_rom(&resp, sizeof(resp));

	return ret;
}

static s32 receive_tisci_msg_sync_resume_req(void)
{
	struct tisci_msg_sync_resume_req req;
	s32 ret = 0;

	lpm_memset(&req, 0, sizeof(req));

	ret = sproxy_receive_msg_rom(&req, sizeof(req));
	if (ret == 0) {
		if (req.hdr.type != TISCI_MSG_SYNC_RESUME) {
			ret = -EINVAL;
		}
	}

	return ret;
}
#else
static s32 send_tisci_msg_firmware_load(void)
{
	return 0;
}
static s32 receive_tisci_msg_continue_resume_req(void)
{
	return 0;
}
static s32 send_tisci_msg_continue_resume_resp(void)
{
	return 0;
}
static s32 receive_tisci_msg_sync_resume_req(void)
{
	return 0;
}
#endif

static void send_msg_bypass_sms_pll(void)
{
}
static void send_msg_restore_sms_pll(void)
{
}

static s32 wait_for_reset_statz(u32 stat)
{
	u32 val;
	u32 i = 0U;
	s32 ret = -ETIMEDOUT;

	do {
		val = readl(WKUP_CTRL_MMR_BASE + SLEEP_STATUS);
		if ((val & SLEEP_STATUS_MAIN_RESETSTATZ) == stat) {
			ret = 0;
			break;
		}
		delay_1us();
	} while (i++ < RETRY_CNT_MS);

	return ret;
}

static s32 wait_for_tifs_ready(void)
{
	u32 val;
	u32 i = 0U;
	s32 ret = -ETIMEDOUT;

	do {
		val = readl(WKUP_CTRL_MMR_BASE + DS_MAGIC_WORD);
		if (val == DS_MAGIC_WORD_RESUME_ROM) {
			writel(0, WKUP_CTRL_MMR_BASE + DS_MAGIC_WORD);
			ret = 0;
			break;
		}
		delay_1us();
	} while (i++ < RETRY_CNT_MS);

	return ret;
}

static void enable_pll_standby(void)
{
}
static void clear_prepare_sleep_data(void)
{
	lpm_memset(&g_params, 0, sizeof(g_params));
}

/*
 * lpm_populate_prepare_sleep_data() for DM to populate stub local data.
 * it should be called by DM before dm_stub_entry().
 */
void lpm_populate_prepare_sleep_data(struct tisci_msg_prepare_sleep_req *p)
{
	if (p != NULL) {
		lpm_memcpy(&g_params, p, sizeof(g_params));
	} else {
		/* Do nothing*/
	}
}

u8 lpm_get_selected_sleep_mode(void)
{
	return g_params.mode;
}

void lpm_reset_wake_reason_params(void)
{
	/* Reset the wake reason parameters at the start of every lpm iteration */
	g_wkup_params.mode          = TISCI_MSG_VALUE_SLEEP_MODE_INVALID;
	g_wkup_params.wake_pin      = TISCI_MSG_VALUE_LPM_WAKE_PIN_INVALID;
	g_wkup_params.wake_source   = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_INVALID;
}

s32 dm_stub_entry(void)
{
	u32 reg;

	lpm_trace_init(SFALSE);

	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_START);

	/* Unlock wkup_ctrl_mmr region 2 & 6 */
	ctrlmmr_unlock(WKUP_CTRL_MMR_BASE, 2);
	ctrlmmr_unlock(WKUP_CTRL_MMR_BASE, 6);

	/* Unlock mcu_ctrl_mmr region 0,2 */
	ctrlmmr_unlock(MCU_CTRL_MMR_BASE, 0);
	ctrlmmr_unlock(MCU_CTRL_MMR_BASE, 2);

	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_MMR_UNLOCK);

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
		for (u32 i = 0U; i < num_main_plls_save_rstr; i++) {
			pll_save(main_plls_save_rstr[i]);
		}
		pll_save(&mcu_pll);

		/* Save TCM (DM stub) contents to DDR */
		(void) memcpy((void *) dm_stub_arr, (const void *) TCMB_BASE_ADDR, TCM_SIZE);

		/* Write back all contents of cache in main memory */
		cache_wb_data();
	}

	/* Load the magic words to latch CANUART status register values */
	if (load_magic_words_through_mmr() != 0) {
		lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_LOAD_MAGIC_WORDS);
		lpm_abort();
	} else {
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_LOAD_MAGIC_WORDS);
	}

	if (enter_ddr_low_power_mode() != 0) {
		lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DDR_SR_ENTER);
		lpm_abort();
	} else {
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DDR_SR_ENTER);
	}

	if (g_params.mode != TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
		if (set_usb_reset_isolation() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_USB_RST_ISO);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_USB_RST_ISO);
		}

		/* Disable all LPSCs in MAIN except Debug, Always ON */
		if (disable_main_lpsc(main_lpscs_phase1, num_main_lpscs_phase1) != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_MAIN_LPSC);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_MAIN_LPSC);
		}

		/* Bypass and disable all PLL in MAIN_PLLCTRL except clock for Debug, PLL0, PLL15 */
		bypass_main_pll();
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_BYPASS_MAIN_PLL);
	}

	wait_for_debug();

	/* Configure selected wake sources with writes to WKUP0_EN IN WKUP_CTRL */
	config_wake_sources();
	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_CONFIG_WAKE_SRC);

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR) {
		/* Enter IO Only plus DDR low power mode */
		enter_io_only_ddr();
	}

	if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
		/* Disable remaining MAIN LPSCs for debug */
		if (disable_main_lpsc(main_lpscs_phase2, num_main_lpscs_phase2) != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_MAIN_LPSC2);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_MAIN_LPSC2);
		}

		/* Modify WKUP_CLKSEL in WKUP_CTRL to use MCU_PLL instead of MAIN PLL */
		writel(WKUP_CLKSEL_MCU, WKUP_CTRL_MMR_BASE + WKUP_CLKSEL);

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_WKUP_CLKSEL_MCU);

		if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) {
			/* Configure GPIO Clock mux in suspend path */
			config_gpio_clk_mux(MCU_CTRL_MMR_CFG0_MCU_GPIO_CLKSEL_CLK_32K);
		}

		/* Enable GPIO wake up */
		enable_gpio_wake_up();

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_CONF_CLK_MUXES);

		/* Set WKUP_CTRL DS_DM_RESET.mask to isolate DM from MAIN domain reset */
		writel(DS_RESET_MASK, WKUP_CTRL_MMR_BASE + DS_DM_RESET);

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_DS_RST_MASK);

		/* Write to WKUP_CTRL DS_MAIN.por_pdoff bits to
		 * place Main domain in reset, gate PD_SMS and PD_DEBUG
		 */
		writel(DS_MAIN_OFF, WKUP_CTRL_MMR_BASE + DS_MAIN);

		if (wait_for_reset_statz(0) != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DS_MAIN_OFF);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DS_MAIN_OFF);
		}
	}

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) {
		/* Disable MCU Domain LPSCs, PDs */
		if (disable_mcu_domain() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_MCU_DOM);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_MCU_DOM);
		}
	}

	if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
		/* Set WKUP_CTRL.RST_CTRL.main_reset_iso_done_z to 1 to
		 * mask main reset in case of RESET_REQz wakeup
		 */
		reg = readl(WKUP_CTRL_MMR_BASE + RST_CTRL);
		reg |= RST_CTRL_MAIN_RST_ISO_DONE_Z;
		writel(reg, WKUP_CTRL_MMR_BASE + RST_CTRL);
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_RST_ISO_DONE);
	} else {
		/* Send TISCI message to bypass SMS PLL */
		send_msg_bypass_sms_pll();
	}

	/* Use WKUP_CTRL.WKUP_WWD0_CTRL to gate clock to RTI */
	writel(WWD_STOP, WKUP_CTRL_MMR_BASE + WKUP_WWD0_CTRL);
	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_MSK_WWD0_CTRL);

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) {
		pll_save(&mcu_pll);
		pll_disable(&mcu_pll);
		lpm_trace_init(STRUE);
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_BYPASS_MCU_PLL);

		/* Set MCU_MMR.MCU_PLL_CLKSEL.clkloss_switch_en to 0
		 *     to avoid clk switch
		 */
		reg = readl(MCU_CTRL_MMR_BASE + MCU_PLL_CLKSEL);
		reg &= ~MCU_PLL_CLKSEL_CLKLOSS_SWTCH_EN;
		writel(reg, MCU_CTRL_MMR_BASE + MCU_PLL_CLKSEL);
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_CLR_CLKLOSS_EN);

		/* Set MCU_MMR.HFOSC0_CTRL.pd_c to power off of HFOSC PD */
		reg = readl(MCU_CTRL_MMR_BASE + HFOSC0_CTRL);
		reg |= HFOSC0_CTRL_PD_C;
		writel(reg, MCU_CTRL_MMR_BASE + HFOSC0_CTRL);
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_HFOSC_PD_C);

		/* Set OSC_CG_ON_WFI bit in WKUP_CTRL.PMCTRL_MOSC */
		reg = readl(WKUP_CTRL_MMR_BASE + PMCTRL_MOSC);
		reg |= PMCTRL_MOSC_OSC_CG_ON_WFI | PMCTRL_MOSC_SETUP_TIME;
		writel(reg, WKUP_CTRL_MMR_BASE + PMCTRL_MOSC);
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_OSC_CG_WFI);
	}

	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_PRE_WFI);

	/* Enable clock gating of legacy peripherals */
	clock_gate_legacy_peripherals(STRUE);

	/* Enter WFI */
	enter_WFI();
	/* Enable global interrupt */
	enable_intr();

	/* Disable clock gating of legacy peripherals */
	clock_gate_legacy_peripherals(SFALSE);

	if (g_wkup_params.wake_source != TISCI_MSG_VALUE_LPM_WAKE_SOURCE_INVALID) {
		lpm_seq_trace_val(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_WAKE_EVENT, g_wkup_params.wake_source);
	} else {
		lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_WAKE_EVENT);
	}

	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_POST_WFI);

	/* Start resume */
	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) {
		/* Clear OSC_CG_ON_WFI bit in WKUP_CTRL.PMCTRL_MOSC */
		reg = readl(WKUP_CTRL_MMR_BASE + PMCTRL_MOSC);
		reg &= ~PMCTRL_MOSC_OSC_CG_ON_WFI;
		writel(reg, WKUP_CTRL_MMR_BASE + PMCTRL_MOSC);

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_CLR_OSC_CG_WFI);

		/* Clear MCU_MMR.HFOSC0_CTRL.pd_c and set MCU_MMR.MCU_PLL_CLKSEL.clkloss_switch_en */
		reg = readl(MCU_CTRL_MMR_BASE + HFOSC0_CTRL);
		reg &= ~HFOSC0_CTRL_PD_C;
		writel(reg, MCU_CTRL_MMR_BASE + HFOSC0_CTRL);

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_CLR_HFOSC_PD_C);

		reg = readl(MCU_CTRL_MMR_BASE + MCU_PLL_CLKSEL);
		reg |= MCU_PLL_CLKSEL_CLKLOSS_SWTCH_EN;
		writel(reg, MCU_CTRL_MMR_BASE + MCU_PLL_CLKSEL);

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_SET_CLKLOSS_EN);

		if (pll_restore(&mcu_pll) != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_RESTORE_MCU_PLL);
			lpm_abort();
		} else {
			lpm_trace_init(SFALSE);
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_RESTORE_MCU_PLL);
		}

		/* Config GPIO Clk mux in resume path */
		config_gpio_clk_mux(MCU_CTRL_MMR_CFG0_MCU_GPIO_CLKSEL_MCU_SYSCLK0);
	}

	/* Use WKUP_CTRL.WKUP_WWD0_CTRL to ungate clock to RTI */
	writel(WWD_RUN, WKUP_CTRL_MMR_BASE + WKUP_WWD0_CTRL);

	lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_UNMSK_WWD0_CTRL);

	if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
		/* Disable WKUP IO Daisy Chain and IO Isolation */

		/* Write 0xdee51ee5 to WKUP DS_MAGIC_WORD to indicate resume path for TIFS ROM */
		writel(DS_MAGIC_WORD_RESUME_TIFS,
		       WKUP_CTRL_MMR_BASE + DS_MAGIC_WORD);

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_SET_MGC_WRD);

		/* Write to WKUP_CTRL DS_MAIN.por_pdoff bits to release Main domain reset */
		writel(DS_MAIN_ON, WKUP_CTRL_MMR_BASE + DS_MAIN);
	}

	wait_for_debug();

	if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
		if (wait_for_reset_statz(SLEEP_STATUS_MAIN_RESETSTATZ) != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_WAIT_MAIN_RST);
			lpm_abort();
		}

		/* Set DM LPSC to enabled as early as possible as JTAG will not connect until this is done. */
		if (enable_dm_lpsc() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_MAIN_DM_LPSC_EN);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_MAIN_DM_LPSC_EN);
		}

		/* Modify WKUP_CLKSEL in WKUP_CTRL to use SMS_PLL instead of MCU PLL */
		writel(WKUP_CLKSEL_MAIN, WKUP_CTRL_MMR_BASE + WKUP_CLKSEL);

		/* Configure additional MCU PSCs to return to pre-DeepSleep state */
		if (enable_mcu_lpsc() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_EN_MCU_LPSC);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_EN_MCU_LPSC);
		}

		/* Poll on WKUP DS_MAGIC_WORD for 0x00d5d02e that indicates
		 * TIFS ROM has completed and execution can continue.
		 */
		if (wait_for_tifs_ready() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_WAIT_TIFS);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_WAIT_TIFS);
		}

		/* Clear WKUP_CTRL DS_DM_RESET.mask to remove isolation of DM
		 * from MAIN domain reset
		 */
		writel(DS_RESET_UNMASK, WKUP_CTRL_MMR_BASE + DS_DM_RESET);
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DS_RST_UNMASK);

		if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
			/* Set WKUP_CTRL.RST_CTRL.main_reset_iso_done_z to 0 to
			* unmask main reset
			*/
			reg = readl(WKUP_CTRL_MMR_BASE + RST_CTRL);
			reg &= ~RST_CTRL_MAIN_RST_ISO_DONE_Z;
			writel(reg, WKUP_CTRL_MMR_BASE + RST_CTRL);
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_RST_ISO_DONE);
		}

		/* Send TISCI ROM Boot image message containing location
		 * and boot address to load FS stub from SPS Memory
		 * TISCI_MSG_FIRMWARE_LOAD
		 */
		if (send_tisci_msg_firmware_load() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_FS_STUB_LD);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_FS_STUB_LD);
		}

		/* Wait for TISCI Message to indicate DDR restore can resume */
		if (receive_tisci_msg_continue_resume_req() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_TISCI_CONT_RES);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_TISCI_CONT_RES);
		}

		/* Configure additional MAIN PLLs and PSCs for EMIF operation */
		if (enable_main_remain_pll() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_MAIN_PLLS);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_MAIN_PLLS);
		}

		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_DDR_RST_ISO);
	} else {
		/* Send TISCI message to restore SMS PLL */
		send_msg_restore_sms_pll();

		/* Relock all PLLs as described in Copy of
		 * PSC_PLL_config_LPM_0p5_ch.xlsx
		 */
		enable_pll_standby();
	}

	/* Disable GPIO wake up */
	disable_gpio_wake_up();

	if (exit_ddr_low_power_mode() != 0) {
		lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DDR_SR_EXIT);
		lpm_abort();
	} else {
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DDR_SR_EXIT);
	}

	if ((g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP) || (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY)) {
		if (release_usb_reset_isolation() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_USB_RST_ISO);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_DIS_USB_RST_ISO);
		}

		/* Send TISCI Message to TIFS to indicate DDR is active and
		 * resume can proceed, include address of TIFS context
		 */
		if (send_tisci_msg_continue_resume_resp() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_RESP_CONT_RES);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_RESP_CONT_RES);
		}

		/* Wait for TISCI_MSG_SYNC_RESUME msg */
		if (receive_tisci_msg_sync_resume_req() != 0) {
			lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_TISCI_SYNC_RES);
			lpm_abort();
		} else {
			lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_TISCI_SYNC_RES);
		}
	}

	lpm_update_wkup_pin_and_mode();

	clear_prepare_sleep_data();

	/* Return to standard firmware in DDR */
	return 0;
}

s32 dm_stub_resume(void)
{
	/* Unlock wkup_ctrl_mmr region 2 & 6 */
	ctrlmmr_unlock(WKUP_CTRL_MMR_BASE, 2U);
	ctrlmmr_unlock(WKUP_CTRL_MMR_BASE, 6U);

	lpm_trace_init(SFALSE);

	/* Update the wake up source */
	g_wkup_params.wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_CAN_IO;
	lpm_update_wkup_pin_and_mode();

	/* Restore MCU domain PLLs */
	if (pll_restore(&mcu_pll) != 0) {
		lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_RESTORE_MCU_PLL);
		lpm_abort();
	} else {
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_RESTORE_MCU_PLL);
	}

	/* Restore MAIN domain PLLs */
	if (enable_main_remain_pll() != 0) {
		lpm_seq_trace_fail(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_MAIN_PLLS);
		lpm_abort();
	} else {
		lpm_seq_trace(TRACE_PM_ACTION_LPM_SEQ_DM_STUB_EN_MAIN_PLLS);
	}

	clear_prepare_sleep_data();

	/* Return to standard firmware in DDR */
	return 0;
}

void dm_stub_irq_handler(void)
{
	u32 int_num;
	int i;

	g_wkup_params.wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_INVALID;

	/* Add an additional delay for oscillator to stabilize. */
	for (i = 0; i < 10; i++) {
		delay_1us();
	}

	int_num = vim_get_intr_number();

	for (i = 0; i < WAKEUP_SOURCE_MAX; i++) {
		if (soc_wake_sources_data[i].int_num == int_num) {
			g_wkup_params.wake_source = soc_wake_sources_data[i].source_id;
			break;
		}
	}

	if (g_params.mode == TISCI_MSG_VALUE_SLEEP_MODE_MCU_ONLY) {
		/* Check whether the interrupt source is MCU IPC */
		if (int_num == MCU_IPC_INTERRUPT_NUMBER) {
			g_wkup_params.wake_source = TISCI_MSG_VALUE_LPM_WAKE_SOURCE_MCU_IPC;
			/* Clear the ipc set and ipc src bits */
			writel(MCU_CTRL_MMR_IPC_CLR0_CLEAR, MCU_CTRL_MMR_BASE + MCU_CTRL_MMR_IPC_CLR0);
		}
	}

	vim_set_intr_enable(int_num, INTR_DISABLE);
	vim_clear_intr(int_num);

	disable_wake_sources();
	vim_irq_complete();
}

void lpm_get_wake_info(struct tisci_msg_lpm_wake_reason_resp *wkup_params)
{
	wkup_params->wake_pin = g_wkup_params.wake_pin;
	wkup_params->wake_source = g_wkup_params.wake_source;
	wkup_params->mode = g_wkup_params.mode;
}
