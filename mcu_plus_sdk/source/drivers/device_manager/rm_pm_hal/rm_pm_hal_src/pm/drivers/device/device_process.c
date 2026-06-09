/*
 * System Firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 * Copyright (C) 2020-2026, Texas Instruments Incorporated
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

#include <device.h>
#include <device_pm.h>
#include <osal/osal_core.h>
#include <tisci/pm/tisci_pm_device.h>
#include <types/errno.h>
#include <lib/trace.h>
#include <lib/mmr_lock.h>
#include <device_prepare.h>
#include <soc/host_idx_mapping.h>
#include <pm.h>
#include <psc.h>
#include <string.h>

/* Maximum valid hardware state value */
#define MAX_HW_STATE_VALUE           (2U)

/* Number of devices that fit in one 32-bit word (32 bits / 2 bits per device) */
#define DEVICES_PER_WORD             (16U)

/* Number of bits used to encode each device state */
#define BITS_PER_DEVICE              (2U)

s32 set_device_handler(u32 *msg_recv)
{
	struct tisci_msg_set_device_req *req =
		(struct tisci_msg_set_device_req *) msg_recv;
	struct tisci_msg_set_device_resp *resp =
		(struct tisci_msg_set_device_resp *) msg_recv;
	struct device *dev = NULL;
	u32 id = req->id;
	u8 state = req->state;
	u32 flags = req->hdr.flags;
	u8 host_id = req->hdr.host;
	sbool enable, retention;
	s32 ret = SUCCESS;
	u32 current_device_state;
	u8 host_idx;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_SET_DEVICE);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID, id);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_VAL, state);

	resp->hdr.flags = 0U;

	mmr_unlock_all();

	ret = device_prepare_exclusive(host_id, id, &host_idx, &dev);
	if (ret == SUCCESS) {
		switch (state) {
		case TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF:
			enable = SFALSE;
			retention = SFALSE;
			break;
		case TISCI_MSG_VALUE_DEVICE_SW_STATE_RETENTION:
			enable = SFALSE;
			retention = STRUE;
			break;
		case TISCI_MSG_VALUE_DEVICE_SW_STATE_ON:
			enable = STRUE;
			retention = STRUE;
			break;
		default:
			pm_trace(TRACE_PM_ACTION_INVALID_STATE, state);
			ret = -EINVAL;
			break;
		}
	}

	if (ret == SUCCESS) {
		if ((flags & TISCI_MSG_FLAG_DEVICE_EXCLUSIVE) != 0UL) {
			/* Make sure no one else has the device enabled */
			u64 mask = DEV_FLAG_ENABLED_MASK;
			u64 enabled;
			/* It's ok if we already have the device enabled */
			mask &= (u64) ~DEV_FLAG_ENABLED(host_idx);
			/* It's also ok if the device in on due to power up en */
			mask &= (u64) ~DEV_FLAG_POWER_ON_ENABLED;
			enabled = (dev->flags & mask) >> DEV_FLAG_ENABLED_BIT;
			if (enabled != 0UL) {
				u8 i;
				/*
				 * Note, rather than trying to fit the enabled
				 * bit field in the trace message, just pick
				 * single host to include.
				 */
				for (i = 0U; i < (sizeof(enabled) * 8U); i++) {
					if ((enabled & 1UL) != 0UL) {
						break;
					}
					enabled >>= 1UL;
				}
#ifdef CONFIG_TRACE
				{
					u32 enabled_host_id = TRACE_PM_VAL_EXCLUSIVE_BUSY_EHOST_ID_MASK >>
							      TRACE_PM_VAL_EXCLUSIVE_BUSY_EHOST_ID_SHIFT;
					/*
					 * Do a reverse lookup. Find host ID from
					 * host index.
					 */
					if (i != (sizeof(enabled) * 8U)) {
						u8 j;
						for (j = 0U; j < soc_host_indexes_sz; j++) {
							if (soc_host_indexes[j] == i) {
								enabled_host_id = j;
								break;
							}
						}
					}
					pm_trace(TRACE_PM_ACTION_EXCLUSIVE_BUSY,
						 (((u32) id << (u32) TRACE_PM_VAL_EXCLUSIVE_BUSY_DEVICE_ID_SHIFT) &
						  (u32) TRACE_PM_VAL_EXCLUSIVE_BUSY_DEVICE_ID_MASK) |
						 (((u32) host_id << (u32) TRACE_PM_VAL_EXCLUSIVE_BUSY_RHOST_ID_SHIFT) &
						  (u32) TRACE_PM_VAL_EXCLUSIVE_BUSY_RHOST_ID_MASK) |
						 (((u32) enabled_host_id << (u32) TRACE_PM_VAL_EXCLUSIVE_BUSY_EHOST_ID_SHIFT) &
						  (u32) TRACE_PM_VAL_EXCLUSIVE_BUSY_EHOST_ID_MASK));
				}
#endif
				ret = -EINVAL;
			}
		}
	}

	if (ret == SUCCESS) {
		if ((flags & TISCI_MSG_FLAG_DEVICE_WAKE_ENABLED) != 0UL) {
			/* Not supported */
		}

		if ((flags & TISCI_MSG_FLAG_DEVICE_EXCLUSIVE) != 0UL) {
			/* Only this host may modify device */
			dev->exclusive = host_idx;
		} else {
			/* Allow any host to modify device */
			dev->exclusive = 0U;
		}

		if ((flags & TISCI_MSG_FLAG_DEVICE_RESET_ISO) != 0UL) {
			device_set_reset_iso(dev, STRUE);
		} else {
			device_set_reset_iso(dev, SFALSE);
		}

		/* Ordering to void unnecessary PD transations */
		if (retention) {
			device_set_retention(dev, retention);
		}
		device_set_state(dev, host_idx, enable);
		if (!retention) {
			device_set_retention(dev, retention);
		}

		/* Check the device state after processing device_set_state function */
		current_device_state = device_get_state(dev);
		if (state == TISCI_MSG_VALUE_DEVICE_SW_STATE_ON) {
			if (current_device_state != TISCI_MSG_VALUE_DEVICE_HW_STATE_ON) {
				ret = -EFAIL;
			}
		} else if ((state == TISCI_MSG_VALUE_DEVICE_SW_STATE_RETENTION) || (state == TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF)) {
			if (current_device_state == TISCI_MSG_VALUE_DEVICE_HW_STATE_TRANS) {
				/* Device with multiple psc's might be in transition state during the requested state is off/retention because of
				 * some psc's sibling devices might be on which keep that psc's on, this results in mixed state of psc's
				 * which is an exception to overcome with this exception below condition is written.
				 */
				if (((struct dev_data *) (get_dev_data(dev)))->soc.psc_idx != PSC_DEV_MULTIPLE) {
					ret = -EFAIL;
				}
			}
		} else {
			ret = -EFAIL;
		}
	}

	mmr_lock_all();

	return ret;
}

s32 get_device_handler(u32 *msg_recv)
{
	struct tisci_msg_get_device_req *req =
		(struct tisci_msg_get_device_req *) msg_recv;
	struct tisci_msg_get_device_resp *resp =
		(struct tisci_msg_get_device_resp *) msg_recv;
	struct device *dev = NULL;
	u32 id = req->id;
	s32 ret = SUCCESS;
	u8 host_idx;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_GET_DEVICE);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID, id);

	resp->hdr.flags = 0U;

	mmr_unlock_all();

	ret = device_prepare_nonexclusive(req->hdr.host, id, &host_idx, &dev);
	if (ret == SUCCESS) {
		u32 context_loss_count;
		u32 resets;
		u8 programmed_state;
		u8 current_state;

		context_loss_count = device_get_context_loss_count(dev);
		resets = device_get_resets(dev);

		if ((dev->flags & DEV_FLAG_ENABLED(host_idx)) != 0UL) {
			programmed_state = TISCI_MSG_VALUE_DEVICE_SW_STATE_ON;
		} else if ((dev->flags & DEV_FLAG_RETENTION) != 0UL) {
			programmed_state = TISCI_MSG_VALUE_DEVICE_SW_STATE_RETENTION;
		} else {
			programmed_state = TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF;
		}

		switch (device_get_state(dev)) {
		case 0:
			current_state = TISCI_MSG_VALUE_DEVICE_HW_STATE_OFF;
			break;
		case 1:
			current_state = TISCI_MSG_VALUE_DEVICE_HW_STATE_ON;
			break;
		default:
			current_state = TISCI_MSG_VALUE_DEVICE_HW_STATE_TRANS;
			break;
		}

		if (device_get_reset_iso(dev)) {
			resp->hdr.flags = TISCI_MSG_FLAG_DEVICE_RESET_ISO;
		}

		resp->context_loss_count        = context_loss_count;
		resp->resets                    = resets;
		resp->programmed_state          = programmed_state;
		resp->current_state             = current_state;
	}

	mmr_lock_all();

	return ret;
}

s32 set_device_resets_handler(u32 *msg_recv)
{
	struct tisci_msg_set_device_resets_req *req =
		(struct tisci_msg_set_device_resets_req *) msg_recv;
	struct tisci_msg_set_device_resets_resp *resp =
		(struct tisci_msg_set_device_resets_resp *) msg_recv;
	struct device *dev = NULL;
	u32 id = req->id;
	u32 resets = req->resets;
	s32 ret = SUCCESS;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_SET_DEVICE_RESETS);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID, id);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_VAL, resets);

	resp->hdr.flags = 0U;

	mmr_unlock_all();

	ret = device_prepare_exclusive(req->hdr.host, id, NULL, &dev);

	if (ret == SUCCESS) {
		if (resets <= 3U) {
			ret = SUCCESS;
		} else {
			pm_trace(TRACE_PM_ACTION_INVALID_STATE, resets);
			ret = EFAIL;
		}
	}

	if (ret == SUCCESS) {
		device_set_resets(dev, resets);
	}

	mmr_lock_all();

	return ret;
}

s32 device_drop_powerup_ref_handler(u32 *msg_recv)
{
	struct tisci_msg_device_drop_powerup_ref_resp *resp =
		(struct tisci_msg_device_drop_powerup_ref_resp *) msg_recv;
	s32 ret = SUCCESS;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_DEVICE_DROP_POWERUP_REF);

	resp->hdr.flags = 0U;

	mmr_unlock_all();

	devices_drop_power_up_ref();

	mmr_lock_all();

	return ret;
}

#ifdef CONFIG_GET_DEVICE_MULTIPLE
s32 get_device_multiple_handler(u32 *msg_recv)
{
	struct tisci_msg_get_device_multiple_req *req =
		(struct tisci_msg_get_device_multiple_req *) msg_recv;
	struct tisci_msg_get_device_multiple_resp *resp =
		(struct tisci_msg_get_device_multiple_resp *) msg_recv;
	s32 ret = SUCCESS;
	u16 total_devices = soc_device_count;
	u16 start_device_id = req->start_device_id;
	u16 end_device_id;
	u16 dev_id;
	u32 bitmap_idx;
	u32 hw_state;
	u32 word_idx;
	u32 bit_pos;

	pm_trace(TRACE_PM_ACTION_MSG_RECEIVED, TISCI_MSG_GET_DEVICE_MULTIPLE);
	pm_trace(TRACE_PM_ACTION_MSG_PARAM_DEV_CLK_ID, start_device_id);

	resp->hdr.flags = 0U;

	mmr_unlock_all();

	(void) memset(resp->device_state_bitmap, 0, sizeof(resp->device_state_bitmap));

	if (start_device_id >= total_devices) {
		resp->count = 0U;
		resp->remaining = 0U;
		pm_trace(TRACE_PM_ACTION_BAD_DEVICE, start_device_id);
		ret = -EFAIL;
	}

	if (ret == SUCCESS) {
		end_device_id = (u16) (start_device_id + TISCI_MSG_MAX_DEVICE_IDS_MULTIPLE - 1U);
		if (end_device_id >= total_devices) {
			end_device_id = (u16) (total_devices - 1U);
			resp->remaining = 0U;
		} else {
			resp->remaining = (u16) (total_devices - end_device_id - 1U);
		}

		resp->count = (u16) (end_device_id - start_device_id + 1U);

		bitmap_idx = 0U;
		for (dev_id = start_device_id; dev_id <= end_device_id; dev_id++) {
			const struct dev_data *data = soc_device_data_arr[dev_id];

			if (data == NULL) {
				hw_state = 0U;
			} else {
				hw_state = device_get_state(&soc_devices[dev_id]);
				if (hw_state > MAX_HW_STATE_VALUE) {
					hw_state = 3U;
				}
			}

			word_idx = bitmap_idx / DEVICES_PER_WORD;
			bit_pos = (bitmap_idx % DEVICES_PER_WORD) * BITS_PER_DEVICE;
			resp->device_state_bitmap[word_idx] |= (hw_state << bit_pos);
			bitmap_idx++;
		}
	}

	mmr_lock_all();

	return ret;
}
#endif /* CONFIG_GET_DEVICE_MULTIPLE */
