/*
 * System Firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 * Copyright (C) 2015-2025, Texas Instruments Incorporated
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

#include <clk.h>
#include <device.h>
#include <clk_mux.h>
#include <types/errno.h>
#include <limits.h>
#include <stddef.h>
#include <types.h>
#include <lib/container_of.h>
#include <lib/trace.h>
#include <types/array_size.h>

/** Maximum number of times to walk the clock tree in LPM handlers */
#define LPM_CLK_MAX_TRIES               10

u32 clk_value_set_freq(struct clk *clkp, u32 target_hz,
		       u32 min_hz __attribute__((unused)),
		       u32 max_hz __attribute__((unused)),
		       sbool query, sbool *changed)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);

	if (!query) {
		*changed = STRUE;
		soc_clock_values[clk_data_p->freq_idx] = target_hz;
	}

	return target_hz;
}

u32 clk_value_get_freq(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);

	return soc_clock_values[clk_data_p->freq_idx];
}

static sbool clk_input_notify_freq(struct clk	*clkp __attribute__((unused)),
				   u32		parent_freq __attribute__(
					   (unused)),
				   sbool	query __attribute__((unused)))
{
	return SFALSE;
}

static sbool clk_input_set_state(struct clk	*clkp __attribute__((unused)),
				 sbool		enabled __attribute__(
					 (unused)))
{
	return STRUE;
}

static u32 clk_input_get_state(struct clk *clkp __attribute__((unused)))
{
	return CLK_HW_STATE_ENABLED;
}

const struct clk_drv clk_drv_input = {
	.set_freq	= clk_value_set_freq,
	.get_freq	= clk_value_get_freq,
	.notify_freq	= clk_input_notify_freq,
	.set_state	= clk_input_set_state,
	.get_state	= clk_input_get_state,
};

u32 clk_get_parent_freq(struct clk *clkp)
{
	const struct clk_parent *p = NULL;
	struct clk *parent_clk = NULL;
	u32 ret = 0;

	if (clkp != NULL) {
		p = clk_get_parent(clkp);
	}

	if (p && p->cdiv) {
		parent_clk = clk_lookup((clk_idx_t) p->clk);
	}

	if (parent_clk != NULL) {
		ret = clk_get_freq(parent_clk) / p->cdiv;
	}

	return ret;
}

sbool clk_notify_freq(struct clk *clkp, u32 parent_freq_hz, sbool query)
{
	const struct clk_data *clk_data_p;
	sbool ret = STRUE;

	if (clkp->freq_change_block_count != 0U) {
		ret = SFALSE;
	}

	clk_data_p = clk_get_data(clkp);

	if (ret) {
		const struct clk_range *range;
		range = clk_get_range(clk_data_p->range_idx);
		if ((parent_freq_hz < range->min_hz) ||
		    (parent_freq_hz > range->max_hz)) {
			ret = SFALSE;
		}
	}

	if (ret) {
		if (clk_data_p->drv->notify_freq != NULL) {
			ret = clk_data_p->drv->notify_freq(clkp, parent_freq_hz, query);
		} else {
			ret = clk_notify_children_freq(clkp, parent_freq_hz, query);
		}
	}

	if (ret && !query) {
		clkp->flags &= (u8) ~CLK_FLAG_CACHED;
	}

	return ret;
}

sbool clk_notify_sibling_freq(struct clk *clkpp, struct clk *parent,
			      u32 parent_freq, sbool query)
{
	struct clk *clkp = clkpp;
	clk_idx_t i;
	clk_idx_t pid = clk_id(parent);
	clk_idx_t id = clkp ? clk_id(clkp) : CLK_ID_NONE;
	sbool status = STRUE;


	/* We must unfortunately walk clock list to find children */
	for (i = 0U; i < soc_clock_count; i++) {
		const struct clk_parent *p;

		/* Skip over clk */
		if (i == id) {
			continue;
		}

		clkp = clk_lookup(i);
		if (!clkp || ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U)) {
			continue;
		}

		p = clk_get_parent(clkp);
		if (!p || (p->clk != pid)) {
			continue;
		}

		if (!clk_notify_freq(clkp, parent_freq / p->cdiv, query)) {
			status = SFALSE;
			break;
		}
	}

	return status;
}

sbool clk_notify_children_freq(struct clk *parent, u32 parent_freq,
			       sbool query)
{
	/*
	 * Reuse sibling walk function, it just avoids whatever clock is in
	 * clk and matches whatever clocks are children of parent.
	 */
	return clk_notify_sibling_freq(parent, parent, parent_freq, query);
}

u32 clk_generic_set_freq_parent(struct clk *clkp, struct clk *parent,
				u32 target_hz, u32 min_hz,
				u32 max_hz, sbool query,
				sbool *changed, u32 d)
{
	u32 new_target, new_min, new_max;
	u32 new_parent_freq = 0;
	u32 ret;

	/* Make sure target fits within out clock frequency type */
	if (((u32) ULONG_MAX / d) < min_hz) {
		ret = 0;
	} else {
		new_min = min_hz * d;
		new_target = target_hz * d;
		new_max = max_hz * d;

		if (new_min < min_hz) {
			ret = 0;
		} else {
			/* Cap overflow in target and max */
			if (new_target < new_min) {
				new_target = (u32) ULONG_MAX;
			}

			if (new_max < new_target) {
				new_max = (u32) ULONG_MAX;
			}

			new_parent_freq = clk_set_freq(parent, new_target, new_min, new_max,
						       STRUE, changed);

			if (0U == new_parent_freq) {
				ret = 0;
			} else {
				/* Check that any siblings can handle the new freq */
				if (*changed && !(clk_notify_sibling_freq(clkp, parent, new_parent_freq,
									  STRUE))) {
					ret = 0;
				} else {
					if (query) {
						ret = new_parent_freq / d;
					} else {
						/* Actually perform the frequency change */
						clk_set_freq(parent, new_target, new_min, new_max, SFALSE, changed);

						/* Notify new rate to siblings */
						if (*changed) {
							clk_notify_sibling_freq(clkp, parent, new_parent_freq, SFALSE);
						}

						ret = new_parent_freq / d;
					}
				}
			}
		}
	}

	return ret;
}

static u32 clk_generic_set_freq(struct clk *clkp,
				u32 target_hz,
				u32 min_hz,
				u32 max_hz, sbool query,
				sbool *changed)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	const struct clk_parent *p = clk_get_parent(clkp);
	u32 ret = 0;

	*changed = SFALSE;

	if (p && (clk_data_p->flags & CLK_DATA_FLAG_MODIFY_PARENT_FREQ)) {
		struct clk *parent;
		parent = clk_lookup((clk_idx_t) p->clk);
		if (parent != NULL) {
			ret = clk_generic_set_freq_parent(clkp, parent,
							  target_hz,
							  min_hz, max_hz,
							  query, changed,
							  p->cdiv);
		}
	} else {
		u32 freq = clk_get_freq(clkp);
		if ((freq >= min_hz) && (freq <= max_hz)) {
			ret = freq;
		}
	}

	return ret;
}
#ifdef CONFIG_TRACE
static void clk_set_freq_trace(struct clk *clkp, u32 freq_hz, sbool failed)
#else
static void clk_set_freq_trace(struct clk *clkp __attribute__((unused)), u32 freq_hz, sbool failed)
#endif
{
	u16 trace_act = TRACE_PM_ACTION_CLOCK_SET_RATE;
	u32 val = freq_hz;
	u32 exp_val = 0;

	if (failed) {
		trace_act |= TRACE_PM_ACTION_FAIL;
	}

	while (val > TRACE_PM_VAL_MAX_FREQ) {
		val >>= 1;
		exp_val += 1U;
	}

	pm_trace((u8) trace_act,
		 (val << TRACE_PM_VAL_CLOCK_VAL_SHIFT) |
		 (exp_val << TRACE_PM_VAL_CLOCK_EXP_SHIFT) |
		 ((u32) (clk_id(clkp) << TRACE_PM_VAL_CLOCK_ID_SHIFT) &
		  TRACE_PM_VAL_CLOCK_ID_MASK));
}

u32 clk_set_freq(struct clk *clkp, u32 target_hz,
		 u32 min_hz, u32 max_hz, sbool query,
		 sbool *changed)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	u32 ret;

	*changed = SFALSE;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if ((clk_data_p->flags & CLK_DATA_FLAG_BLOCK_FREQ_CHANGE) != 0U) {
		ret = 0U;
	} else if (clk_data_p->drv->set_freq != NULL) {
		ret = clk_data_p->drv->set_freq(clkp, target_hz, min_hz,
						max_hz, query, changed);
	} else {
		ret = clk_generic_set_freq(clkp, target_hz, min_hz, max_hz,
					   query, changed);
	}

	clk_set_freq_trace(clkp, target_hz, ret == 0U);

	return ret;
}

u32 clk_get_freq(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	u32 ret;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if (clk_data_p->drv->get_freq != NULL) {
		ret = clk_data_p->drv->get_freq(clkp);
	} else {
		ret = clk_get_parent_freq(clkp);
	}

	return ret;
}

u32 clk_get_state(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	u32 ret = CLK_HW_STATE_DISABLED;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = CLK_HW_STATE_DISABLED;
	} else if (clk_data_p->drv->get_state != NULL) {
		ret = clk_data_p->drv->get_state(clkp);
	} else {
		const struct clk_parent *p;
		p = clk_get_parent(clkp);
		if (p != NULL) {
			struct clk *clkp_parent;
			clkp_parent = clk_lookup((clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				ret = clk_get_state(clkp_parent);
			}
		}
	}

	return ret;
}

sbool clk_set_state(struct clk *clkp, sbool enable)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	sbool ret;

	if (clk_data_p != NULL) {
		if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
			/* defer action */
			ret = STRUE;
		} else if (clk_data_p->drv->set_state == NULL) {
			ret = STRUE;
		} else {
			ret = clk_data_p->drv->set_state(clkp, enable);
		}
	} else {
		ret = STRUE;
	}

	return ret;
}

sbool clk_get(struct clk *clkp)
{
	sbool ret = STRUE;

	if (0U == clkp->ref_count) {
		const struct clk_parent *p;
		struct clk *clkp_parent = NULL;

		p = clk_get_parent(clkp);
		if (p != NULL) {
			clkp_parent = clk_lookup((clk_idx_t) p->clk);
		}

		if (clkp_parent != NULL) {
			ret = clk_get(clkp_parent);
		}

		if (ret) {
			ret = clk_set_state(clkp, STRUE);
			if (!ret && clkp_parent) {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE |
					 TRACE_PM_ACTION_FAIL, clk_id(clkp));
				clk_put(clkp_parent);
			} else {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE,
					 clk_id(clkp));
			}
		}
	}

	if (ret) {
		clkp->ref_count++;
	}

	return ret;
}

void clk_put(struct clk *clkp)
{
	if (--clkp->ref_count == 0U) {
		const struct clk_parent *p;

		p = clk_get_parent(clkp);
		clk_set_state(clkp, SFALSE);
		pm_trace(TRACE_PM_ACTION_CLOCK_DISABLE, clk_id(clkp));
		if (p != NULL) {
			struct clk *clkp_parent;
			clkp_parent = clk_lookup((clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				clk_put(clkp_parent);
			}
		}
	}
}

#ifdef CONFIG_PM_CLK_SSC
u32 clk_set_ssc(struct clk *clkp, u32 modfreq_hz, u32 mod_depth, u8 spread_type, sbool enable)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	u32 ret;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = 1U;       /* Fail: Clock not initialized */
	} else if ((clk_data_p->flags & CLK_DATA_FLAG_ALLOW_SSC_CHANGE) == 0U) {
		ret = 1U;       /* Fail: Hardware does not support SSC on this clock */
	} else if (clk_data_p->drv->set_ssc == NULL) {
		ret = 1U;       /* Fail: No driver support for SSC */
	} else {
		/* Success: Call hardware-specific driver to set SSC parameters */
		ret = (clk_data_p->drv->set_ssc(clkp, modfreq_hz, mod_depth,
						spread_type, enable) == 0) ? 0U : 1U;
	}

	return ret;
}

u32 clk_get_ssc(struct clk *clkp, struct ssc_data *ssc_datap)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	u32 ret = 0U;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = 1U;       /* Fail: Clock not initialized */
	} else if ((clk_data_p->flags & CLK_DATA_FLAG_ALLOW_SSC_CHANGE) == 0U) {
		ret = 1U;       /* Fail: Hardware does not support SSC on this clock */
	} else if (clk_data_p->drv->get_ssc == NULL) {
		ret = 1U;       /* Fail: No driver support for SSC */
	} else {
		/* Success: Call hardware-specific driver to get SSC parameters */
		clk_data_p->drv->get_ssc(clkp, ssc_datap);
	}

	return ret;
}
#endif

void clk_freq_change_allow(struct clk *clkp)
{
	clkp->freq_change_block_count--;
}

void clk_freq_change_block(struct clk *clkp)
{
	clkp->freq_change_block_count++;
}

static s32 clk_register_clock(struct clk *clkp, const struct clk_data *clk_data_p)
{
	s32 ret = SUCCESS;
	struct clk *clkp_parent = NULL;
	const struct clk_parent *p;

	p = clk_get_parent(clkp);
	if (p != NULL) {
		clkp_parent = clk_lookup((clk_idx_t) p->clk);
	}
	if ((clkp_parent != NULL) && ((clkp_parent->flags & CLK_FLAG_INITIALIZED) == 0U)) {
		ret = -EDEFER;
	}

	if ((ret == SUCCESS) && (clk_data_p->drv->init != NULL)) {
		ret = clk_data_p->drv->init(clkp);
	}

	if (ret == SUCCESS) {
		clkp->flags |= CLK_FLAG_INITIALIZED;
		if (clkp->ref_count != 0U) {
			if (!clk_set_state(clkp, STRUE)) {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE |
					 TRACE_PM_ACTION_FAIL, clk_id(clkp));
			} else {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE,
					 clk_id(clkp));
			}
		}
		if (clk_data_p->drv->get_state != NULL) {
			if (clk_data_p->drv->get_state(clkp) != CLK_HW_STATE_DISABLED) {
				clkp->flags |= CLK_FLAG_PWR_UP_EN;
			}
		}
	}

	return ret;
}

void clk_drop_pwr_up_en(void)
{
	clk_idx_t i;

	for (i = 0U; i < soc_clock_count; i++) {
		if ((soc_clocks[i].flags & CLK_FLAG_PWR_UP_EN) != 0U) {
			clk_put(soc_clocks + i);
			soc_clocks[i].flags &= (u8) ~CLK_FLAG_PWR_UP_EN;
		}
	}
}

s32 clk_deinit_pm_devgrp(u8 pm_devgrp)
{
	s32 ret = SUCCESS;
	u32 i;
	u32 clk_id_start;
	u32 clk_id_end = 0U;
	u8 pm_devgrp_val = pm_devgrp;

	clk_id_start = soc_devgroups[pm_devgrp_val].clk_idx;

	if (pm_devgrp_val >= soc_devgroup_count) {
		ret = -EINVAL;
	} else if (pm_devgrp_val == (soc_devgroup_count - 1U)) {
		/* Last devgrp's last clock id is the same as last of all clock ids */
		clk_id_end = soc_clock_count;
	} else {
		/* Chosen devgrp's last clock id is next devgrp's first clock id */

		/* Loop through all the devgrp till we find valid devgrp */
		pm_devgrp_val = pm_devgrp_val + 1U;
		while (pm_devgrp_val < soc_devgroup_count) {
			/* Check if next dev grp is valid devgrp*/
			if (soc_devgroups[pm_devgrp_val].clk_idx != 0U) {
				clk_id_end = soc_devgroups[pm_devgrp_val].clk_idx;
				break;
			}
			pm_devgrp_val = pm_devgrp_val + 1U;
		}

		/* If no valid devgrp is found,clock id is the same as last of all clock ids */
		if (pm_devgrp_val == soc_devgroup_count) {
			clk_id_end = soc_clock_count;
		}
	}

	/*
	 * Loop through all the clocks in selected device group.
	 * First pass make sure any clock with the PWR_UP_EN bit set gets
	 * a matching put call and the flag cleared. Clocks can be in this
	 * state if initialization for the given domain is in the deferred state.
	 */
	if (ret == SUCCESS) {
		for (i = clk_id_start; i < clk_id_end; i++) {
			struct clk *clkp = soc_clocks + i;

			/* Clear the power up flag */
			if ((clkp->flags & CLK_FLAG_PWR_UP_EN) != 0U) {
				clk_put(clkp);
				soc_clocks[i].flags &= (u8) ~CLK_FLAG_PWR_UP_EN;
			}
		}
		/*
		* Second pass clear the initialized flag and check that the ref_count
		* is zero as expected.
		*/
		for (i = clk_id_start; i < clk_id_end; i++) {
			struct clk *clkp = soc_clocks + i;

			/* Clear the initialized flag */
			clkp->flags &= (u8) ~CLK_FLAG_INITIALIZED;
		}
	}
	return ret;
}

#ifdef CONFIG_LPM_CLK
static s32 clk_suspend_save(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	s32 ret = SUCCESS;

	if (clk_data_p != NULL) {
		if ((clkp->flags & CLK_FLAG_SUSPENDED) == 0U) {
			if (clk_data_p->drv->suspend_save != NULL) {
				ret = clk_data_p->drv->suspend_save(clkp);
				/* Mark clock as suspended to avoid duplicate operations */
				clkp->flags |= CLK_FLAG_SUSPENDED;
			} else {
				/* Mark clock as resumed if no handler is provided */
				clkp->flags |= CLK_FLAG_SUSPENDED;
				ret = SUCCESS;
			}
		}
	} else {
		ret = SUCCESS;
	}

	return ret;
}

static s32 clk_resume_restore(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	const struct clk_parent *p = NULL;
	struct clk *parent_clk = NULL;
	s32 ret = SUCCESS;

	p = clk_get_parent(clkp);
	if (p != NULL) {
		parent_clk = clk_lookup((clk_idx_t) p->clk);
	}

	if (parent_clk != NULL) {
		/* If parent is still suspended, defer until it has resumed. */
		if ((parent_clk->flags & CLK_FLAG_SUSPENDED) == CLK_FLAG_SUSPENDED) {
			ret = -EDEFER;
		}
	}

	if ((ret != -EDEFER) && ((clkp->flags & CLK_FLAG_SUSPENDED) == CLK_FLAG_SUSPENDED)) {
		if (clk_data_p->drv->resume_restore != NULL) {
			ret = clk_data_p->drv->resume_restore(clkp);
			/* Clear suspended flag to avoid duplicate operations */
			clkp->flags &= ~CLK_FLAG_SUSPENDED;
		} else {
			/* Mark clock as resumed if no handler is provided */
			clkp->flags &= ~CLK_FLAG_SUSPENDED;
			ret = SUCCESS;
		}
	}

	return ret;
}

s32 clks_suspend(void)
{
	u32 i;
	s32 ret = SUCCESS;
	u32 clock_count = soc_clock_count;
	u8 max_tries = LPM_CLK_MAX_TRIES;
	sbool done, error;

	do {
		done = STRUE;
		error = SFALSE;

		for (i = 1u; i < clock_count; i++) {
			struct clk *clkp = soc_clocks + i;

			ret = clk_suspend_save(clkp);

			if (ret == -EDEFER) {
				done = SFALSE;
			} else if (ret != SUCCESS) {
				error = STRUE;
			} else {
				/* Do nothing */
			}
		}

		/* Avoid getting stuck forever, bound the number of loops */
		max_tries--;
	} while (!done && !error && (max_tries != 0U));

	if (max_tries == 0U) {
		ret = -ETIMEDOUT;
	} else {
		ret = SUCCESS;
	}

	return ret;
}

s32 clks_resume(void)
{
	u32 i;
	s32 ret = SUCCESS;
	u32 clock_count = soc_clock_count;
	u8 max_tries = 10;
	sbool done, error;

	do {
		done = STRUE;
		error = SFALSE;

		for (i = 1u; i < clock_count; i++) {
			struct clk *clkp = soc_clocks + i;

			ret = clk_resume_restore(clkp);

			if (ret == -EDEFER) {
				done = SFALSE;
			} else if (ret != SUCCESS) {
				error = STRUE;
			} else {
				/* Do nothing */
			}
		}

		/* Avoid getting stuck forever, bound the number of loops */
		max_tries--;
	} while (!done && !error && (max_tries != 0U));

	if (max_tries == 0U) {
		ret = -ETIMEDOUT;
	} else {
		ret = SUCCESS;
	}

	return ret;
}
#endif

s32 clk_init(void)
{
	sbool progress;
	sbool contents;
	sbool enabled = STRUE;
	s32 ret = SUCCESS;
	u32 i;
	u32 clock_count = soc_clock_count;

	for (i = 0U; i < soc_devgroup_count; i++) {
		devgrp_t devgrp;

		/* Translate compressed internal representation to bitfield */
		if (i == PM_DEVGRP_DMSC) {
			devgrp = DEVGRP_DMSC;
		} else {
			devgrp = (devgrp_t) BIT(i - 1U);
		}

		/* First disabled devgroup, stop at this clock index */
		if (enabled && !pm_devgroup_is_enabled(devgrp)) {
			clock_count = soc_devgroups[i].clk_idx;
			enabled = SFALSE;
		}
	}

	contents = SFALSE;
	progress = SFALSE;

	/* Loop through all the clocks to initialize them */
	for (i = 0U; i < clock_count; i++) {
		struct clk *clkp = soc_clocks + i;
		const struct clk_data *clk_data_p = soc_clock_data + i;
		s32 curr;

		if (((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) && (clk_data_p->drv != NULL)) {
			contents = STRUE;
			curr = clk_register_clock(clkp, clk_data_p);
			if (curr != -EDEFER) {
				progress = STRUE;
				if (curr != SUCCESS) {
					ret = curr;
				}
			}
		}
	}

	if (progress) {
		for (i = 0U; i < clock_count; i++) {
			if ((soc_clocks[i].flags & CLK_FLAG_PWR_UP_EN) != 0U) {
				if (!clk_get(soc_clocks + i)) {
					/* clk_get failed for one of the clocks */
					ret = -EFAIL;
					break;
				}
			}
		}
	} else if (contents) {
		/* We processed at least one clock but didn't make progress */
		ret = -EDEFER;
	} else {
		/* We didn't process any clocks */
		ret = SUCCESS;
	}

	return ret;
}
