/*
 * Ring Accelerator
 *
 * Ring Accelerator management infrastructure
 *
 * Copyright (C) 2018-2025, Texas Instruments Incorporated
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

#include <types/sbool.h>
#include <types/short_types.h>
#include <types/address_types.h>
#include <types/errno.h>
#include <lib/io.h>
#include <lib/trace.h>

#include <config.h>
#include <hosts.h>

#include <osal/osal_core.h>
#include <tisci/rm/tisci_rm_ra.h>
#include <rm.h>

#include <rm_core.h>
#include <rm_ra.h>

#include <ra_inst.h>
#include <ra_cfg.h>
#include <udmap_inst.h>

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
#include <security/rm_int_firewall.h>
#include <tifs/src/security/secure_rm/sec_rm.h>
#endif

/*
 * RA Common Configuration Register Macros
 */
#define RA_CFG_RING_BASE(RING)                     (((u32) RING) * ((u32) 0x100U))
#define RA_CFG_MON_BASE(MONITOR)                   ((MONITOR) * 0x1000U)

#define RA_GCFG_OVRFLOW_DISABLE                    (0x0000FFFFu)
#define RA_GCFG_OVRFLOW                            (0x00000020u)

#define RA_GCFG_ERROR_EVENT                        (0x00000040u)

/*
 * RA Ring Configuration Register Macros
 */
#define RA_CFG_RING_BA_LO                          (0x00000040u)
#define RA_CFG_RING_BA_HI                          (0x00000044u)
#define RA_CFG_RING_SIZE                           (0x00000048u)
#define RA_CFG_RING_EVENT                          (0x0000004Cu)
#define RA_CFG_RING_ORDERID                        (0x00000050u)

#define RA_CFG_MON_CONTROL                         (0x00000000U)

/*
 * RA Ring Configuration Register Fields
 */
#define RA_CFG_RING_BA_LO_ADDR_LO_MASK             (0xFFFFFFFFu)
#define RA_CFG_RING_BA_LO_ADDR_LO_SHIFT            (0x00000000u)

#define RA_CFG_RING_BA_HI_ADDR_HI_MASK             (0x0000FFFFu)
#define RA_CFG_RING_BA_HI_ADDR_HI_SHIFT            (0x00000000u)
#define RA_CFG_RING_BA_HI_ADDR_ASEL_MASK           (0x000F0000U)
#define RA_CFG_RING_BA_HI_ADDR_ASEL_SHIFT          (0x00000010U)

#define RA_CFG_RING_SIZE_QMODE_MASK                (0xC0000000u)
#define RA_CFG_RING_SIZE_QMODE_SHIFT               (0x0000001Eu)
#define RA_CFG_RING_SIZE_ELSIZE_MASK               (0x07000000u)
#define RA_CFG_RING_SIZE_ELSIZE_SHIFT              (0x00000018u)
#define RA_CFG_RING_SIZE_ELCNT_MASK                (0x000FFFFFu)
#define RA_CFG_RING_SIZE_ELCNT_SHIFT               (0x00000000u)
#define RA_CFG_RING_SIZE_ELCNT_MAX                 (0x000FFFFFu)

#define RA_CFG_RING_ORDERID_REPLACE_MASK           (0x00000010u)
#define RA_CFG_RING_ORDERID_REPLACE_SHIFT          (0x00000004u)
#define RA_CFG_RING_ORDERID_ORDERID_MASK           (0x0000000Fu)
#define RA_CFG_RING_ORDERID_ORDERID_SHIFT          (0x00000000u)
#define RA_CFG_RING_ORDERID_ORDERID_MAX            (15u)

#define RA_CFG_MON_CONTROL_EVT_MASK                (0xFFFF0000U)
#define RA_CFG_MON_CONTROL_EVT_SHIFT               (16U)

#define RA_GENERIC_EVENT_EVT_MASK                  (0x0000FFFFu)
#define RA_GENERIC_EVENT_EVT_SHIFT                 (0x00000000u)
#define RA_GENERIC_EVENT_EVT_RESETVAL              (0x0000FFFFu)

/**
 * \brief Structure for local data for RM RA valid param selection per ring type
 *
 * \param mask bitwise OR of all TISCI_MSG_VALUE_RM_RING_*_VALID supported
 */
struct ra_valid_masks_local {
	u32 mask;
};

/**
 * \brief Local list of valid param mask per ring type.
 */
static const struct ra_valid_masks_local local_rm_ra_valid_masks[RA_NUM_RING_TYPES] = {
#ifdef CONFIG_RM_RA_NAV_RING
	[RA_STANDARD_RING] = {
		TISCI_MSG_VALUE_RM_RING_ADDR_LO_VALID |
		TISCI_MSG_VALUE_RM_RING_ADDR_HI_VALID |
		TISCI_MSG_VALUE_RM_RING_COUNT_VALID |
		TISCI_MSG_VALUE_RM_RING_MODE_VALID |
		TISCI_MSG_VALUE_RM_RING_SIZE_VALID |
		TISCI_MSG_VALUE_RM_RING_ORDER_ID_VALID |
		TISCI_MSG_VALUE_RM_RING_VIRTID_VALID
	},
#else
	[RA_STANDARD_RING] = { 0U },
#endif
#ifdef CONFIG_RM_RA_DMSS_RING
	[RA_DMSS_RING] =     {
		TISCI_MSG_VALUE_RM_RING_ADDR_LO_VALID |
		TISCI_MSG_VALUE_RM_RING_ADDR_HI_VALID |
		TISCI_MSG_VALUE_RM_RING_COUNT_VALID |
		TISCI_MSG_VALUE_RM_RING_ASEL_VALID
	},
#else
	[RA_DMSS_RING] =     { 0U },
#endif
};

/**
 * \brief Get RA instance mapped to provided device ID
 *
 * \param id RA device ID
 *
 * \param trace_action RA action to trace
 *
 * \return Pointer to RA instance, NULL if no instance mapped to ID
 */
static const struct ra_instance *ra_get_inst(u16 id, u8 trace_action)
{
	const struct ra_instance *inst = NULL;
	u32 i;
	u8 trace_action_val_p = trace_action;

	for (i = 0U; i < ra_inst_count; i++) {
		if (ra_inst[i].id == id) {
			inst = &ra_inst[i];

			if (rm_core_validate_devgrp(inst->id, inst->devgrp) !=
			    SUCCESS) {
				/*
				 * Return NULL instance if instance devgrp
				 * doesn't align with boardcfg devgrp
				 */
				inst = NULL;
			}
			break;
		}
	}

	if (inst == NULL) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_DEVICE_ID,
		     id);

	return inst;
}

/**
 * \brief Get RA instance mapped to provided Navigator Subsystem root
 *        device ID and ring_type
 *
 * \param root_id SoC device ID of Navigator Subsystem in which the RA
 *                device is located
 *
 * \param ring_type Ring Type for properly matching the RA instance
 *
 * \param trace_action RA action to trace
 *
 * \return Pointer to RA instance, NULL if no instance mapped to root ID
 */
static const struct ra_instance *ra_get_inst_from_root_id_ring_type(u16 root_id,
								    u8	ring_type,
								    u8	trace_action)
{
	const struct ra_instance *inst = NULL;
	u8 i;
	u8 trace_action_val_p = trace_action;

	for (i = 0; i < ra_inst_count; i++) {
		/* A properly matched RA in NAVSS must also have matching ring_type */
		if ((ra_inst[i].root_id == root_id) &&
		    (ra_inst[i].ring_types[0].type == ring_type)) {
			inst = &ra_inst[i];

			if (rm_core_validate_devgrp(inst->id, inst->devgrp) !=
			    SUCCESS) {
				/*
				 * Return NULL instance if instance devgrp
				 * doesn't align with boardcfg devgrp
				 */
				inst = NULL;
			}
			break;
		}
	}

	if (inst == NULL) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_SS_DEVICE_ID,
		     root_id);

	return inst;
}

/**
 * \brief Validate ring monitor index against the board cfg resasg data
 *
 * \param inst RA instance
 *
 * \param host Host ID
 *
 * \param index Ring monitor index
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if monitor index is valid, else -EINVAL
 */
static s32 ra_check_monitor_index_range(const struct ra_instance *inst, u8 host,
					u16 index, u8 trace_action)
{
	s32 r = -EINVAL;
	u8 i;
	u8 trace_action_val_p = trace_action;

	for (i = 0U; i < inst->n_ring_mon_type; i++) {
		if ((index >= inst->ring_mon_types[i].start) &&
		    (index <= inst->ring_mon_types[i].end)) {
			r = rm_core_resasg_validate_resource(
				host,
				inst->ring_mon_utype,
				index);
			break;
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_INDEX,
		     index);

	return r;
}

/**
 * \brief Validate ring monitor index against the board cfg resasg data
 *
 * \param inst RA instance
 *
 * \param host Host ID
 *
 * \param index Ring monitor index
 *
 * \param utype Returns the ring utype if not NULL
 *
 * \param trace_action RA action to trace
 *
 * \param ring_type Returns ring type (DMSS/NAV) if not NULL
 *
 * \return SUCCESS if ring index is valid, else -EINVAL
 */
static s32 ra_check_index_range(const struct ra_instance *inst, u8 host,
				u16 index, u16 *utype, u8 trace_action,
				u8 *ring_type)
{
	s32 r = -EINVAL;
	u8 i;
	u8 trace_action_val_p = trace_action;

	for (i = 0U; i < inst->n_ring_type; i++) {
		if ((index >= inst->ring_types[i].start) &&
		    (index <= inst->ring_types[i].end)) {
			r = rm_core_resasg_validate_resource(
				host,
				inst->ring_types[i].utype,
				index);
			break;
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	} else {
		if (utype != NULL) {
			*utype = inst->ring_types[i].utype;
		}
		if (ring_type != NULL) {
			*ring_type = inst->ring_types[i].type;
		}
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_INDEX,
		     index);

	return r;
}

/**
 * \brief Validate ring virtid
 *
 * \param inst RA instance
 *
 * \param host TISCI message Host ID
 *
 * \param virtid Ring Virt ID
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if host ID can program ring virtid, else -EINVAL
 */
#ifdef CONFIG_RM_RA_NAV_RING
static s32 ra_check_virtid(const struct ra_instance *inst, u8 host, u16 virtid,
			   u8 trace_action)
{
	s32 r;
	u8 trace_action_val_p = trace_action;

	r = rm_core_resasg_validate_resource(host, inst->virtid_utype,
					     virtid);
	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_VIRTID,
		     virtid);

	return r;
}
#endif

/**
 * \brief Validates the ring mode
 *
 * \param mode Ring mode
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if ring mode is validity, else -EINVAL
 */
static s32 ra_validate_mode(u8 mode, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (mode) {
	case TISCI_MSG_VALUE_RM_RING_MODE_RING:
		/* Also catches mode reset value */
		break;
	case TISCI_MSG_VALUE_RM_RING_MODE_MESSAGE:
		break;
	case TISCI_MSG_VALUE_RM_RING_MODE_CREDENTIALS:
		break;
	case TISCI_MSG_VALUE_RM_RING_MODE_QM:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_MODE,
		     mode);

	return r;
}

/**
 * \brief Convert encoded ring size to bytes
 *
 * \param size Encoded ring size
 *
 * \param mode Ring mode
 *
 * \param size_bytes Returns the ring size in bytes
 *
 * \param trace_action RA action to trace
 *
 * \param ring_type Ring type (DMSS/NAV)
 *
 * \return SUCCESS if encoded size is valid, else -EINVAL
 */
static s32 ra_get_size_bytes(u8 size, u8 mode, u32 *size_bytes, u8 trace_action,
			     u8 ring_type)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;
	u8 size_val = size;

	if (ring_type == RA_DMSS_RING) {
		/* DMSS rings are always 8 bytes */
		size_val = TISCI_MSG_VALUE_RM_RING_SIZE_8B;
	}

	switch (size_val) {
	case TISCI_MSG_VALUE_RM_RING_SIZE_4B:
		/* Also catches size reset */
		*size_bytes = 4u;
		break;
	case TISCI_MSG_VALUE_RM_RING_SIZE_8B:
		*size_bytes = 8u;
		break;
	case TISCI_MSG_VALUE_RM_RING_SIZE_16B:
		*size_bytes = 16u;
		break;
	case TISCI_MSG_VALUE_RM_RING_SIZE_32B:
		*size_bytes = 32u;
		break;
	case TISCI_MSG_VALUE_RM_RING_SIZE_64B:
		*size_bytes = 64u;
		break;
	case TISCI_MSG_VALUE_RM_RING_SIZE_128B:
		*size_bytes = 128u;
		break;
	case TISCI_MSG_VALUE_RM_RING_SIZE_256B:
		*size_bytes = 256u;
		break;
	default:
		*size_bytes = 0u;
		r = -EINVAL;
		break;
	}

	if ((r == SUCCESS) &&
	    (mode == TISCI_MSG_VALUE_RM_RING_MODE_QM) &&
	    (*size_bytes != 8u)) {
		/* QM modes requires size be 8 bytes */
		r = -EINVAL;
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_SIZE,
		     size_val);

	return r;
}

/**
 * \brief Validates the input base address alignment
 *
 * \param addr_lo 32 LSBs of base address
 *
 * \param size_bytes Ring element size
 *
 * \param mode Ring mode
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if base address alignment is okay, else -EINVAL
 */
static s32 ra_validate_base_addr(u32 addr_lo, u32 size_bytes, u8 mode,
				 u8 trace_action)
{
	s32 r = SUCCESS;
	u32 loc_size_bytes;
	u8 trace_action_val_p = trace_action;

	if (addr_lo != 0U) {
		/*
		 * The base address must be aligned to the element size
		 * of the ring for ring and message modes.  For
		 * credentials and QM modes the ring base address must
		 * be aligned to 2 * element size.
		 */
		if ((mode == TISCI_MSG_VALUE_RM_RING_MODE_CREDENTIALS) ||
		    (mode == TISCI_MSG_VALUE_RM_RING_MODE_QM)) {
			loc_size_bytes = 2u * size_bytes;
		} else {
			loc_size_bytes = size_bytes;
		}

		loc_size_bytes -= 1u;
		if ((addr_lo & loc_size_bytes) != 0u) {
			trace_action_val_p |= TRACE_RM_ACTION_FAIL;
			r = -EINVAL;
		}
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_BA_LO_HI,
		     ((addr_lo >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_BA_LO_LO,
		     (addr_lo & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	return r;
}

/**
 * \brief Validates the ring count
 *
 * \param count Ring count
 *
 * \param mode Ring mode
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if count is valid, else -EINVAL
 */
static s32 ra_validate_count(u32 count, u8 mode, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (count > RA_CFG_RING_SIZE_ELCNT_MAX) {
		r = -EINVAL;
	} else {
		if ((mode ==
		     TISCI_MSG_VALUE_RM_RING_MODE_CREDENTIALS) ||
		    (mode ==
		     TISCI_MSG_VALUE_RM_RING_MODE_QM)) {
			/*
			 * Count must be even if mode is credentials or
			 * QM
			 */
			if ((count & 0x1u) == 1u) {
				r = -EINVAL;
			}
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_COUNT_HI,
		     ((count >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_COUNT_LO,
		     (count & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	return r;
}

/**
 * \brief Validate ring order ID
 *
 * \param order_id Ring order ID
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if order ID is valid, else -EINVAL
 */
static s32 ra_validate_order_id(u8 order_id, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (order_id > RA_CFG_RING_ORDERID_ORDERID_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_ORDERID,
		     order_id);

	return r;
}

/**
 * \brief Validate ring monitor source
 *
 * \param source Ring monitor source
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if monitor source is valid, else -EINVAL
 */
static s32 ra_validate_monitor_source(u8 source, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (source) {
	case TISCI_MSG_VALUE_RM_MON_SRC_ELEM_CNT:
		break;
	case TISCI_MSG_VALUE_RM_MON_SRC_HEAD_PKT_SIZE:
		break;
	case TISCI_MSG_VALUE_RM_MON_SRC_ACCUM_Q_SIZE:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_MONITOR_SOURCE,
		     source);

	return r;
}

/**
 * \brief Validate ring monitor mode
 *
 * \param mode Ring monitor mode
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if monitor mode is valid, else -EINVAL
 */
static s32 ra_validate_monitor_mode(u8 mode, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (mode) {
	case TISCI_MSG_VALUE_RM_MON_MODE_DISABLED:
		break;
	case TISCI_MSG_VALUE_RM_MON_MODE_PUSH_POP:
		break;
	case TISCI_MSG_VALUE_RM_MON_MODE_THRESHOLD:
		break;
	case TISCI_MSG_VALUE_RM_MON_MODE_WATERMARK:
		break;
	case TISCI_MSG_VALUE_RM_MON_MODE_STARVATION:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_RING_MONITOR_MODE,
		     mode);

	return r;
}

/**
 * \brief Clear a ring's OES value programmed by ROM
 *
 * \param inst Pointer to RA instance
 *
 * \param ring Ring index
 *
 * \return SUCCESS if no clear performed or if clear succeeds, else -EFAIL
 */
static s32 ra_clear_rom_oes(const struct ra_instance *inst, u16 ring)
{
	s32 r = SUCCESS;
	u8 i;
	struct ra_used_mapping *used_mapping;
	mapped_addr_t maddr;
	u32 ring_base;
	u32 evt_reg;

	for (i = 0U; i < inst->n_rom_usage; i++) {
		used_mapping = &inst->rom_usage[i];
		if ((used_mapping->cleared == SFALSE) &&
		    (used_mapping->ring == ring)) {
			evt_reg = RA_GENERIC_EVENT_EVT_RESETVAL;
			maddr = rm_core_map_region(inst->cfg->base);
			ring_base = maddr + RA_CFG_RING_BASE(ring);
			r = writel_verified(evt_reg, ring_base +
					    RA_CFG_RING_EVENT);
			rm_core_unmap_region();
			if (r == SUCCESS) {
				used_mapping->cleared = STRUE;
			}
			break;
		}
	}

	return r;
}

/**
 * \brief Gets the global event in a RA event register
 *
 * \param inst RA instance
 *
 * \param oes_index OES event index
 *
 * \return Global event value within the OES index's event register
 */
static u16 ra_get_evt(const struct ra_instance *inst, u16 oes_index)
{
	mapped_addr_t maddr;
	u32 evt_addr;
	u32 evt_reg;
	u32 shift = RA_GENERIC_EVENT_EVT_SHIFT;
	u32 mask = RA_GENERIC_EVENT_EVT_MASK;

	if (oes_index == inst->err_evt_index) {
		maddr = rm_core_map_region(inst->gcfg->base);
		evt_addr = maddr + RA_GCFG_ERROR_EVENT;
	} else if ((oes_index >= inst->ring_mon_oes_offset) &&
		   (oes_index < (inst->ring_mon_oes_offset +
				 inst->n_ring_mon))) {
		/*
		 * Breaking the evt_addr computation into multiple statements to
		 * comply with MISRA.ETYPE.COMP.CAST.EXPL.WIDER.2012
		 */
		u32 offset = RA_CFG_MON_CONTROL;

		offset += RA_CFG_MON_BASE(((u32) oes_index) - ((u32) inst->ring_mon_oes_offset));

		maddr = rm_core_map_region(inst->mon->base);
		evt_addr = maddr + offset;
		shift = RA_CFG_MON_CONTROL_EVT_SHIFT;
		mask = RA_CFG_MON_CONTROL_EVT_MASK;
	} else {
		maddr = rm_core_map_region(inst->cfg->base);
		evt_addr = maddr + RA_CFG_RING_BASE(oes_index) +
			   RA_CFG_RING_EVENT;
	}

	evt_reg = readl(evt_addr);
	rm_core_unmap_region();

	return (u16) rm_fext(evt_reg, shift, mask);
}

/**
 * \brief Gets the global event within a ring's RING_EVT register
 *
 * \param host Host ID of processing entity attempting to get the event.
 *
 * \param id SoC device ID in which the RA OES index resides
 *
 * \param oes_index Flattened OES register index.  Needs to be decoded based on
 *                  RA instance ring and monitor offsets.
 *
 * \param evt Returns the global event value within the OES index's RING_EVT
 *            or RING_CONTROL register
 *
 * \return SUCCESS if global event retrieved successfully, else error
 */
static s32 ra_get_ring_evt(u8 host, u16 id, u16 oes_index, u16 *evt)
{
	s32 r = SUCCESS;
	const struct ra_instance *inst = NULL;
	u8 trace_action = TRACE_RM_ACTION_RING_OES_GET;

	if (evt == NULL) {
		r = -EINVAL;
	} else {
		inst = ra_get_inst(id, trace_action);
		if (inst == NULL) {
			r = -EINVAL;
		}
	}

	if (r == SUCCESS) {
		if (oes_index == inst->err_evt_index) {
			r = rm_core_resasg_validate_resource(
				host,
				inst->err_evt_utype,
				0u);
		} else if ((oes_index >= inst->gp_ring_oes_offset_pre_sp) &&
			   (oes_index < (inst->gp_ring_oes_offset_pre_sp +
					 inst->n_gp_rings_pre_sp_rings))) {
			r = ra_check_index_range(inst, host, oes_index, NULL,
						 trace_action, NULL);
			if (r == SUCCESS) {
				r = ra_clear_rom_oes(inst, oes_index);
				if (r != SUCCESS) {
					trace_action |= TRACE_RM_ACTION_FAIL;
					r = -EINVAL;
				}
			}
		} else if (((inst->n_gp_rings_post_sp_rings != 0U) &&
			    ((oes_index >= inst->gp_ring_oes_offset_post_sp) &&
			     (oes_index < (inst->gp_ring_oes_offset_post_sp +
					   inst->n_gp_rings_post_sp_rings))))) {
			r = ra_check_index_range(inst, host, oes_index, NULL,
						 trace_action, NULL);
			if (r == SUCCESS) {
				r = ra_clear_rom_oes(inst, oes_index);
				if (r != SUCCESS) {
					trace_action |= TRACE_RM_ACTION_FAIL;
					r = -EINVAL;
				}
			}
		} else if ((oes_index >= inst->ring_mon_oes_offset) &&
			   (oes_index < (inst->ring_mon_oes_offset +
					 inst->n_ring_mon))) {
			r = ra_check_monitor_index_range(inst, host,
							 (oes_index -
							  inst->ring_mon_oes_offset),
							 trace_action);
		} else {
			trace_action |= TRACE_RM_ACTION_FAIL;
			r = -EINVAL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_OES_REG_INDEX,
			     oes_index);
	}

	if (r == SUCCESS) {
		*evt = ra_get_evt(inst, oes_index);
		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_EVENT,
			     *evt);
	}

	return r;
}

/**
 * \brief Sets the global event in a ring's RING_EVT register
 *
 * \param host Host ID of processing entity attempting to set the event.
 *
 * \param id SoC device ID in which the RA OES index resides
 *
 * \param oes_index Flattened OES register index.  Needs to be decoded based on
 *                  RA instance ring and monitor offsets.
 *
 * \param evt Global event value to program into the OES index's RING_EVT
 *            register
 *
 * \return SUCCESS if global event programmed successfully, else error
 */
static s32 ra_set_ring_evt(u8 host, u16 id, u16 oes_index, u16 evt)
{
	s32 r = SUCCESS;
	const struct ra_instance *inst = NULL;
	mapped_addr_t maddr;
	u32 evt_reg;
	u32 evt_addr;
	u8 trace_action = TRACE_RM_ACTION_RING_OES_SET;

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_EVENT,
		     evt);

	inst = ra_get_inst(id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		if (oes_index == inst->err_evt_index) {
			r = rm_core_resasg_validate_resource(
				host,
				inst->err_evt_utype,
				0u);
		} else if ((oes_index >= inst->gp_ring_oes_offset_pre_sp) &&
			   (oes_index < (inst->gp_ring_oes_offset_pre_sp +
					 inst->n_gp_rings_pre_sp_rings))) {
			r = ra_check_index_range(inst, host, oes_index, NULL,
						 trace_action, NULL);
			if (r == SUCCESS) {
				r = ra_clear_rom_oes(inst, oes_index);
				if (r != SUCCESS) {
					trace_action |= TRACE_RM_ACTION_FAIL;
					r = -EINVAL;
				}
			}
		} else if (((inst->n_gp_rings_post_sp_rings != 0U) &&
			    ((oes_index >= inst->gp_ring_oes_offset_post_sp) &&
			     (oes_index < (inst->gp_ring_oes_offset_post_sp +
					   inst->n_gp_rings_post_sp_rings))))) {
			r = ra_check_index_range(inst, host, oes_index, NULL,
						 trace_action, NULL);
			if (r == SUCCESS) {
				r = ra_clear_rom_oes(inst, oes_index);
				if (r != SUCCESS) {
					trace_action |= TRACE_RM_ACTION_FAIL;
					r = -EINVAL;
				}
			}
		} else if ((oes_index >= inst->ring_mon_oes_offset) &&
			   (oes_index < (inst->ring_mon_oes_offset +
					 inst->n_ring_mon))) {
			r = ra_check_monitor_index_range(inst, host,
							 (oes_index -
							  inst->ring_mon_oes_offset),
							 trace_action);
		} else {
			trace_action |= TRACE_RM_ACTION_FAIL;
			r = -EINVAL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_OES_REG_INDEX,
			     oes_index);
	}

	if (r == SUCCESS) {
		/* oes_index verified already at this point */
		if (oes_index == inst->err_evt_index) {
			maddr = rm_core_map_region(inst->gcfg->base);
			evt_reg = rm_fmk(RA_GENERIC_EVENT_EVT_SHIFT,
					 RA_GENERIC_EVENT_EVT_MASK, evt);
			evt_addr = maddr + RA_GCFG_ERROR_EVENT;
		} else if ((oes_index >= inst->ring_mon_oes_offset) &&
			   (oes_index < (inst->ring_mon_oes_offset +
					 inst->n_ring_mon))) {
			maddr = rm_core_map_region(inst->mon->base);
			evt_addr = maddr +
				   (u32) (RA_CFG_MON_BASE(oes_index -
							  inst->ring_mon_oes_offset)) +
				   RA_CFG_MON_CONTROL;
			evt_reg = readl(evt_addr);
			evt_reg &= ~RA_CFG_MON_CONTROL_EVT_MASK;
			evt_reg |= rm_fmk(RA_CFG_MON_CONTROL_EVT_SHIFT,
					  RA_CFG_MON_CONTROL_EVT_MASK, evt);
		} else {
			maddr = rm_core_map_region(inst->cfg->base);
			evt_reg = rm_fmk(RA_GENERIC_EVENT_EVT_SHIFT,
					 RA_GENERIC_EVENT_EVT_MASK, evt);
			evt_addr = maddr + RA_CFG_RING_BASE(oes_index) +
				   RA_CFG_RING_EVENT;
		}

		if (writel_verified(evt_reg, evt_addr) != SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
		rm_core_unmap_region();
	}

	return r;
}

/**
 * \brief Configures the overflow ring to be disabled by default
 *
 * \param base The address of the global configuration register
 *
 * \return SUCCESS if the overflow queue is programmed successfully, else error
 */
#ifndef CONFIG_RM_RA_OVERFLOW_DEFAULT
static s32 ra_set_gcfg_ovrflow_default(soc_phys_addr_t base)
{
	s32 r = SUCCESS;
	mapped_addr_t maddr;
	u32 ovrflow_addr, ovrflow_reg;

	maddr = rm_core_map_region(base);
	ovrflow_addr = maddr + RA_GCFG_OVRFLOW;

	/*
	 * Set the first 16 bits to 0xFFFF
	 */
	ovrflow_reg = readl(ovrflow_addr);
	ovrflow_reg |= RA_GCFG_OVRFLOW_DISABLE;

	if (writel_verified(ovrflow_reg, ovrflow_addr) != SUCCESS) {
		r = -EFAILVERIFY;
	}

	rm_core_unmap_region();

	return r;
}
#endif

/**
 * \brief Configure a ring
 *
 * \param inst RA instance
 *
 * \param msg Pointer to the ring configure TISCI message
 */
static s32 ra_configure(const struct ra_instance		*inst,
			const struct tisci_msg_rm_ring_cfg_req	*msg)
{
	mapped_addr_t maddr;
	u32 ring_base;
	u32 ba_lo_reg;
	u32 ba_hi_reg;
	u32 size_reg;
	u32 order_id_reg;
	sbool write;
	s32 r = SUCCESS;

	maddr = rm_core_map_region(inst->cfg->base);
	ring_base = maddr + RA_CFG_RING_BASE(msg->index);

	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_ADDR_LO_VALID) ==
	    STRUE) {
		ba_lo_reg = rm_fmk(RA_CFG_RING_BA_LO_ADDR_LO_SHIFT,
				   RA_CFG_RING_BA_LO_ADDR_LO_MASK,
				   msg->addr_lo);
		if (writel_verified(ba_lo_reg,
				    ring_base + RA_CFG_RING_BA_LO) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	ba_hi_reg = readl(ring_base + RA_CFG_RING_BA_HI);
	write = SFALSE;

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_RING_ADDR_HI_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		ba_hi_reg &= ~RA_CFG_RING_BA_HI_ADDR_HI_MASK;
		ba_hi_reg |= rm_fmk(RA_CFG_RING_BA_HI_ADDR_HI_SHIFT,
				    RA_CFG_RING_BA_HI_ADDR_HI_MASK,
				    msg->addr_hi);
		write = STRUE;
	}

#ifdef CONFIG_RM_RA_DMSS_RING
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_RING_ASEL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		ba_hi_reg &= ~RA_CFG_RING_BA_HI_ADDR_ASEL_MASK;
		ba_hi_reg |= rm_fmk(RA_CFG_RING_BA_HI_ADDR_ASEL_SHIFT,
				    RA_CFG_RING_BA_HI_ADDR_ASEL_MASK,
				    msg->asel);
		write = STRUE;
	}
#endif

	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(ba_hi_reg,
				    ring_base + RA_CFG_RING_BA_HI) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		size_reg = readl(ring_base + RA_CFG_RING_SIZE);
		write = SFALSE;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_RING_MODE_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		size_reg &= ~RA_CFG_RING_SIZE_QMODE_MASK;
		size_reg |= rm_fmk(RA_CFG_RING_SIZE_QMODE_SHIFT,
				   RA_CFG_RING_SIZE_QMODE_MASK,
				   msg->mode);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_RING_SIZE_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		size_reg &= ~RA_CFG_RING_SIZE_ELSIZE_MASK;
		size_reg |= rm_fmk(RA_CFG_RING_SIZE_ELSIZE_SHIFT,
				   RA_CFG_RING_SIZE_ELSIZE_MASK,
				   msg->size);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_RING_COUNT_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		size_reg &= ~RA_CFG_RING_SIZE_ELCNT_MASK;
		size_reg |= rm_fmk(RA_CFG_RING_SIZE_ELCNT_SHIFT,
				   RA_CFG_RING_SIZE_ELCNT_MASK,
				   msg->count);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(size_reg,
				    ring_base + RA_CFG_RING_SIZE) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_RING_ORDER_ID_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		order_id_reg = rm_fmk(
			RA_CFG_RING_ORDERID_REPLACE_SHIFT,
			RA_CFG_RING_ORDERID_REPLACE_MASK, 1u);
		order_id_reg |= rm_fmk(
			RA_CFG_RING_ORDERID_ORDERID_SHIFT,
			RA_CFG_RING_ORDERID_ORDERID_MASK,
			msg->order_id);
		if (writel_verified(order_id_reg,
				    ring_base + RA_CFG_RING_ORDERID) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	rm_core_unmap_region();
	return r;
}

/**
 * \brief Create a local cfg msg using register values if the register
 *        parameter is specified as not valid
 *
 * \param inst RA instance
 *
 * \param msg Pointer to the received ring cfg message
 *
 * \param loc_msg Pointer to the local ring cfg message
 *
 * \param dev_valid_mask Mask indicating allowed valid bits for this DMA
 */
static void ra_format_local_cfg_msg(
	const struct ra_instance		*inst,
	const struct tisci_msg_rm_ring_cfg_req	*msg,
	struct tisci_msg_rm_ring_cfg_req	*loc_msg,
	u32					dev_valid_mask)
{
	mapped_addr_t maddr;
	u32 ring_base;
	u32 ba_lo_reg;
	u32 ba_hi_reg;
	u32 size_reg;
	u32 order_id_reg = 0U;

	loc_msg->valid_params = msg->valid_params & dev_valid_mask;
	loc_msg->nav_id = msg->nav_id;
	loc_msg->index = msg->index;

	maddr = rm_core_map_region(inst->cfg->base);

	/* The following registers are valid on all devices supported by this
	   driver.  Don't check valid bits */
	ring_base = maddr + RA_CFG_RING_BASE(msg->index);
	ba_lo_reg = readl(ring_base + RA_CFG_RING_BA_LO);
	ba_hi_reg = readl(ring_base + RA_CFG_RING_BA_HI);
	size_reg = readl(ring_base + RA_CFG_RING_SIZE);

	/* The following are valid on some devices so gate with valid bits */
#ifdef CONFIG_RM_RA_NAV_RING
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_ORDER_ID_VALID) ==
	    SFALSE) {
		/* Only read orderid if it exists for this RA in this device */
		if (rm_core_param_is_valid(dev_valid_mask,
					   TISCI_MSG_VALUE_RM_RING_ORDER_ID_VALID) ==
		    STRUE) {
			order_id_reg = readl(ring_base + RA_CFG_RING_ORDERID);
		}
	}
#else
	order_id_reg = 0U;
#endif
	rm_core_unmap_region();

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_ADDR_LO_VALID) ==
	    STRUE) {
		loc_msg->addr_lo = msg->addr_lo;
	} else {
		/* All rings have a ba_lo */
		loc_msg->addr_lo = rm_fext(ba_lo_reg,
					   RA_CFG_RING_BA_LO_ADDR_LO_SHIFT,
					   RA_CFG_RING_BA_LO_ADDR_LO_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_ADDR_HI_VALID) ==
	    STRUE) {
		loc_msg->addr_hi = msg->addr_hi;
	} else {
		/* All rings have a ba_hi */
		loc_msg->addr_hi = rm_fext(ba_hi_reg,
					   RA_CFG_RING_BA_HI_ADDR_HI_SHIFT,
					   RA_CFG_RING_BA_HI_ADDR_HI_MASK);
	}

#ifdef CONFIG_RM_RA_DMSS_RING
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_ASEL_VALID) ==
	    STRUE) {
		loc_msg->asel = msg->asel;
	} else {
		loc_msg->asel = (u8) rm_fext(ba_hi_reg,
					     RA_CFG_RING_BA_HI_ADDR_ASEL_SHIFT,
					     RA_CFG_RING_BA_HI_ADDR_ASEL_MASK);
	}
#endif

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_COUNT_VALID) ==
	    STRUE) {
		loc_msg->count = msg->count;
	} else {
		/* All rings have a count */
		loc_msg->count = (u32) rm_fext(size_reg,
					       RA_CFG_RING_SIZE_ELCNT_SHIFT,
					       RA_CFG_RING_SIZE_ELCNT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_MODE_VALID) ==
	    STRUE) {
		loc_msg->mode = msg->mode;
	} else {
		/* Bits extracted but not used on devices without a MODE */
		loc_msg->mode = (u8) rm_fext(size_reg,
					     RA_CFG_RING_SIZE_QMODE_SHIFT,
					     RA_CFG_RING_SIZE_QMODE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_SIZE_VALID) ==
	    STRUE) {
		loc_msg->size = msg->size;
	} else {
		/* Bits extracted but not used on devices without a ELSIZE */
		loc_msg->size = (u8) rm_fext(size_reg,
					     RA_CFG_RING_SIZE_ELSIZE_SHIFT,
					     RA_CFG_RING_SIZE_ELSIZE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_RING_ORDER_ID_VALID) ==
	    STRUE) {
		loc_msg->order_id = msg->order_id;
	} else {
		loc_msg->order_id = (u8) rm_fext(order_id_reg,
						 RA_CFG_RING_ORDERID_ORDERID_SHIFT,
						 RA_CFG_RING_ORDERID_ORDERID_MASK);
	}

	/*
	 * Always copy message's virtid since reading HW value is not
	 * available.  Validity against board configuration has already been
	 * checked at this point.  Validity against what is allowed by HW is
	 * checked by Secure.
	 */
	loc_msg->virtid = msg->virtid;
}

s32 rm_ra_cfg(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_ring_cfg_req *msg =
		(struct tisci_msg_rm_ring_cfg_req *) msg_recv;
	const struct ra_instance *inst = NULL;
	struct tisci_msg_rm_ring_cfg_req loc_msg;
	u16 utype;
	u32 size_bytes;
	u8 trace_action = TRACE_RM_ACTION_RING_CONFIGURE;
	u32 device_valid_mask;
	u8 ring_type;
	u32 masked_valid_params;

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	u8 hosts[FWL_MAX_PRIVID_SLOTS];
	u8 n_hosts = 0U;
#endif

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = ra_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = ra_check_index_range(inst, msg->hdr.host, msg->index,
					 &utype, trace_action, &ring_type);
	}

	if (r == SUCCESS) {
		device_valid_mask = local_rm_ra_valid_masks[ring_type].mask;
		masked_valid_params = msg->valid_params & device_valid_mask;
	}

#ifdef CONFIG_RM_RA_NAV_RING
	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(masked_valid_params,
				    TISCI_MSG_VALUE_RM_RING_VIRTID_VALID) ==
	     STRUE)) {
		r = ra_check_virtid(inst, msg->hdr.host, msg->virtid,
				    trace_action);
	}
#endif

	if (r == SUCCESS) {
		/*
		 * Create a local version of cfg message with register
		 * values for parameters that were specified as not valid
		 */
		ra_format_local_cfg_msg(inst, msg, &loc_msg, device_valid_mask);
	}

	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(masked_valid_params,
				    TISCI_MSG_VALUE_RM_RING_MODE_VALID) ==
	     STRUE)) {
		r = ra_validate_mode(loc_msg.mode, trace_action);
	}
	if (r == SUCCESS) {
		r = ra_get_size_bytes(loc_msg.size, loc_msg.mode, &size_bytes,
				      trace_action, ring_type);
	}
	if (r == SUCCESS) {
		r = ra_validate_base_addr(loc_msg.addr_lo, size_bytes,
					  loc_msg.mode, trace_action);
	}
	if (r == SUCCESS) {
		r = ra_validate_count(loc_msg.count, loc_msg.mode,
				      trace_action);
	}
	if (r == SUCCESS) {
		r = ra_validate_order_id(loc_msg.order_id, trace_action);
	}

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	if (r == SUCCESS) {
		/* Call Secure RM to configure ring firewalls */

		r = rm_core_get_resasg_hosts(utype,
					     loc_msg.index,
					     &n_hosts,
					     &hosts[0U],
					     FWL_MAX_PRIVID_SLOTS);

		if (r == SUCCESS) {
			r = sec_rm_ra_fwl_cfg(msg_recv, hosts, n_hosts);
		}
	}
#endif

	if (r == SUCCESS) {
		r = ra_configure(inst, &loc_msg);
	}

	return r;
}

s32 rm_ra_mon_cfg(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_ring_mon_cfg_req *msg =
		(struct tisci_msg_rm_ring_mon_cfg_req *) msg_recv;
	const struct ra_instance *inst = NULL;
	u8 trace_action = TRACE_RM_ACTION_RING_MON_CFG;

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	u8 hosts[FWL_MAX_PRIVID_SLOTS];
	u8 n_hosts = 0U;
#endif

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = ra_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = ra_check_monitor_index_range(inst, msg->hdr.host,
						 msg->index, trace_action);
	}

	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_MON_SOURCE_VALID) ==
	     STRUE)) {
		r = ra_validate_monitor_source(msg->source, trace_action);
	}
	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_MON_MODE_VALID) ==
	     STRUE)) {
		r = ra_validate_monitor_mode(msg->mode, trace_action);
	}
	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_MON_QUEUE_VALID) ==
	     STRUE)) {
		/* Validate queue, or ring, index against board cfg */
		r = ra_check_index_range(inst, msg->hdr.host, msg->queue,
					 NULL, trace_action, NULL);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_RING_MONITOR_QUEUE,
			     msg->queue);
	}

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	if (r == SUCCESS) {
		/* Call Secure RM to configure ring monitor firewalls */

		r = rm_core_get_resasg_hosts(inst->ring_mon_utype,
					     msg->index,
					     &n_hosts,
					     &hosts[0U],
					     FWL_MAX_PRIVID_SLOTS);

		if (r == SUCCESS) {
			r = sec_rm_ra_mon_cfg(msg_recv, hosts, n_hosts);
		}
	}
#endif

	return r;
}

s32 rm_ra_validate_ring_index(u16 nav_id, u8 udma_chan_type, u8 host, u16 index)
{
	s32 r = SUCCESS;
	const struct ra_instance *inst;
	u8 trace_action = TRACE_RM_ACTION_RING_VALIDATE_INDEX;
	u8 ring_type = RA_NUM_RING_TYPES;

	switch (udma_chan_type) {
	case UDMAP_TX_CHAN:
	case UDMAP_TX_HCHAN:
	case UDMAP_TX_UHCHAN:
	case UDMAP_TX_ECHAN:
	case UDMAP_RX_CHAN:
	case UDMAP_RX_HCHAN:
	case UDMAP_RX_UHCHAN:
		ring_type = RA_STANDARD_RING;
		break;
	case DMSS_BCDMA_TX_CHAN:
	case DMSS_BCDMA_TX_HCCHAN:
	case DMSS_BCDMA_RX_CHAN:
	case DMSS_BCDMA_RX_HCCHAN:
	case DMSS_BCDMA_BLOCK_COPY_HCCHAN:
	case DMSS_BCDMA_BLOCK_COPY_CHAN:
	case DMSS_PKTDMA_TX_CHAN:
	case DMSS_PKTDMA_RX_CHAN:
		ring_type = RA_DMSS_RING;
		break;
	default:
		trace_action |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	if (r == SUCCESS) {
		inst = ra_get_inst_from_root_id_ring_type(nav_id, ring_type, trace_action);
		if (inst == NULL) {
			r = -EINVAL;
		} else {
			r = ra_check_index_range(inst, host, index, NULL,
						 trace_action, NULL);
		}
	}

	return r;
}

sbool rm_ra_is_managed_resasg_utype(u16 utype)
{
	sbool r = SFALSE;
	u16 i, j;

	for (i = 0u; i < ra_inst_count; i++) {
		for (j = 0u;
		     (j < ra_inst[i].n_ring_type) && (r == SFALSE);
		     j++) {
			if (utype == ra_inst[i].ring_types[j].utype) {
				r = STRUE;
			}
		}

		if (r == SFALSE) {
			if ((utype == ra_inst[i].err_evt_utype) ||
			    (utype == ra_inst[i].ring_mon_utype) ||
			    (utype == ra_inst[i].virtid_utype)) {
				r = STRUE;
			}
		}

		if (r == STRUE) {
			break;
		}
	}

	return r;
}

s32 rm_ra_init(void)
{
	s32 r = SUCCESS;
	u16 i;

	for (i = 0U; i < ra_inst_count; i++) {
		if ((rm_core_validate_devgrp(ra_inst[i].id, ra_inst[i].devgrp) ==
		     SUCCESS) &&
		    (ra_inst[i].initialized == SFALSE)) {
#ifdef CONFIG_RM_IRQ
			ra_inst[i].oes_handler.oes_get = ra_get_ring_evt;
			ra_inst[i].oes_handler.oes_set = ra_set_ring_evt;

			r = rm_irq_oes_src_register(ra_inst[i].id,
						    &ra_inst[i].oes_handler);
#endif
#ifndef CONFIG_RM_RA_OVERFLOW_DEFAULT
			if ((r == SUCCESS) && (ra_inst[i].virtid_utype != 0U)) {
				r = ra_set_gcfg_ovrflow_default(ra_inst[i].gcfg->base);
			}
#endif
			if (r != SUCCESS) {
				break;
			} else {
				ra_inst[i].initialized = STRUE;
			}
		}
	}

	if (r != SUCCESS) {
		rm_trace_sub((TRACE_RM_ACTION_RA_INIT | TRACE_RM_ACTION_FAIL),
			     0U, 0U);
	}

	return r;
}

s32 rm_ra_deinit(devgrp_t devgrp, sbool rom_deinit)
{
	s32 r = -EFAIL;
	u8 i, j;

	for (i = 0U; i < ra_inst_count; i++) {
		if ((rm_core_validate_devgrp(ra_inst[i].id, ra_inst[i].devgrp) ==
		     SUCCESS) &&
		    (ra_inst[i].initialized == STRUE) &&
		    (ra_inst[i].devgrp == devgrp)) {
			ra_inst[i].initialized = SFALSE;

			if (rom_deinit == STRUE) {
				/* Since ROM will be using these resources again, reset the ROM usage flags,
				 * so that way these resources are able to be cleared again during RM init.
				 */
				for (j = 0U; j < (ra_inst[i].n_rom_usage); j++) {
					ra_inst[i].rom_usage[j].cleared = SFALSE;
				}
			}

			r = SUCCESS;
		}
	}

	return r;
}
