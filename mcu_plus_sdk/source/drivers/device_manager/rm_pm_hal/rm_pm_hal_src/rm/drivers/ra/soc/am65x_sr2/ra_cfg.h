/*
 * Ring Accelerator
 *
 * Ring accelerator SoC-specific configuration definitions
 *
 * Data version: 000000_000000
 *
 * Copyright (C) 2019-2026 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */
#ifndef RA_CFG_H
#define RA_CFG_H

/**
 * NAVSS0_RINGACC0 total number of rings before secure proxy rings
 */
#define RA_NAVSS0_RINGACC0_MAX_INDEX_PRE_SP (768U)
/**
 * NAVSS0_RINGACC0 total number of rings after secure proxy rings
 */
#define RA_NAVSS0_RINGACC0_MAX_INDEX_POST_SP (0U)
/**
 * Ring accelerator ring event index for rings before secure proxy rings.
 */
#define RA_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_PRE_SP (0x0U)
/**
 * Ring accelerator ring event index for rings after secure proxy rings.
 */
#define RA_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_POST_SP (0x0U)
/**
 * NAVSS0_RINGACC0 total number of ring monitors
 */
#define RA_NAVSS0_RINGACC0_MONITOR_MAX_INDEX (32U)
/**
 * MCU_NAVSS0_RINGACC0 total number of rings before secure proxy rings
 */
#define RA_MCU_NAVSS0_RINGACC0_MAX_INDEX_PRE_SP (256U)
/**
 * MCU_NAVSS0_RINGACC0 total number of rings after secure proxy rings
 */
#define RA_MCU_NAVSS0_RINGACC0_MAX_INDEX_POST_SP (0U)
/**
 * Ring accelerator ring event index for rings before secure proxy rings.
 */
#define RA_MCU_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_PRE_SP (0x0U)
/**
 * Ring accelerator ring event index for rings after secure proxy rings.
 */
#define RA_MCU_NAVSS0_RINGACC0_RING_OES_EVT_INDEX_OFFSET_POST_SP (0x0U)
/**
 * MCU_NAVSS0_RINGACC0 total number of ring monitors
 */
#define RA_MCU_NAVSS0_RINGACC0_MONITOR_MAX_INDEX (32U)
/**
 * Ring accelerator ring monitor event index.  Must be greater than the max of
 * all SoC RAs max ring index to avoid event indexing conflicts.
 */
#define RA_MON_OES_EVT_INDEX_OFFSET (0x400U)
/**
 * Ring accelerator global control error event index.  Must be greater than
 * the max of all SoC RAs max ring index to avoid event indexing conflicts.
 */
#define RA_GCFG_ERROR_EVT_INDEX (0x800U)
/**
 * Total number of RA instances
 */
#define RA_INST_RANGE (2U)

#endif /* RA_CFG_H */
