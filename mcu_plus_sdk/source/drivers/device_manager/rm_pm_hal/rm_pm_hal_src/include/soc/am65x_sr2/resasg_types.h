/*
 * Resource Management
 *
 * Resource Assignment Subtype definitions
 *
 * Data version: 000000_000000
 *
 * Copyright (C) 2019-2026, Texas Instruments Incorporated
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
#ifndef RESASG_TYPES_H
#define RESASG_TYPES_H

/**
 * Resource assignment type shift
 */
#define RESASG_TYPE_SHIFT (0x0006U)
/**
 * Resource assignment type mask
 */
#define RESASG_TYPE_MASK (0xFFC0U)
/**
 * Resource assignment subtype shift
 */
#define RESASG_SUBTYPE_SHIFT (0x0000U)
/**
 * Resource assignment subtype mask
 */
#define RESASG_SUBTYPE_MASK (0x003FU)
/**
 * Macro to create unique resource assignment types using type and subtype
 */

#define RESASG_UTYPE(type, subtype) \
	(((type << RESASG_TYPE_SHIFT) & RESASG_TYPE_MASK) | \
	 ((subtype << RESASG_SUBTYPE_SHIFT) & RESASG_SUBTYPE_MASK))

/**
 * IA subtypes definitions
 */
#define RESASG_SUBTYPE_IA_VINT (0x000AU)
#define RESASG_SUBTYPE_GLOBAL_EVENT_GEVT (0x000BU)
#define RESASG_SUBTYPE_GLOBAL_EVENT_MEVT (0x000CU)
#define RESASG_SUBTYPE_GLOBAL_EVENT_SEVT (0x000DU)
#define RESASG_SUBTYPES_IA_CNT (0x0004U)

/**
 * IR subtypes definitions
 */
#define RESASG_SUBTYPE_IR_OUTPUT (0x0000U)
#define RESASG_SUBTYPES_IR_CNT (0x0001U)

/**
 * Proxy subtypes definitions
 */
#define RESASG_SUBTYPE_PROXY_PROXIES (0x0000U)
#define RESASG_SUBTYPES_PROXY_CNT (0x0001U)

/**
 * RA subtypes definitions
 */
#define RESASG_SUBTYPE_RA_ERROR_OES (0x0000U)
#define RESASG_SUBTYPE_RA_GP (0x0001U)
#define RESASG_SUBTYPE_RA_UDMAP_RX (0x0002U)
#define RESASG_SUBTYPE_RA_UDMAP_TX (0x0003U)
#define RESASG_SUBTYPE_RA_UDMAP_TX_EXT (0x0004U)
#define RESASG_SUBTYPE_RA_UDMAP_RX_H (0x0005U)
#define RESASG_SUBTYPE_RA_UDMAP_TX_H (0x0007U)
#define RESASG_SUBTYPE_RA_VIRTID (0x000AU)
#define RESASG_SUBTYPE_RA_MONITORS (0x000BU)
#define RESASG_SUBTYPES_RA_CNT (0x0009U)

/**
 * UDMAP subtypes definitions
 */
#define RESASG_SUBTYPE_UDMAP_RX_FLOW_COMMON (0x0000U)
#define RESASG_SUBTYPE_UDMAP_INVALID_FLOW_OES (0x0001U)
#define RESASG_SUBTYPE_GLOBAL_EVENT_TRIGGER (0x0002U)
#define RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG (0x0003U)
#define RESASG_SUBTYPE_UDMAP_RX_CHAN (0x000AU)
#define RESASG_SUBTYPE_UDMAP_RX_HCHAN (0x000BU)
#define RESASG_SUBTYPE_UDMAP_TX_CHAN (0x000DU)
#define RESASG_SUBTYPE_UDMAP_TX_ECHAN (0x000EU)
#define RESASG_SUBTYPE_UDMAP_TX_HCHAN (0x000FU)
#define RESASG_SUBTYPES_UDMAP_CNT (0x0009U)


/**
 * Total number of unique resource types for SoC
 */
#define RESASG_UTYPE_CNT 52U

/**
 * Total number of resource entries allowed for SoC
 */
#define RESASG_ENTRIES_MAX (RESASG_UTYPE_CNT * 5U)

#endif /* RESASG_TYPES_H */
