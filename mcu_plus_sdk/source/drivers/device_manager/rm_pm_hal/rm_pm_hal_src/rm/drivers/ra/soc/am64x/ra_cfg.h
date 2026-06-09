/*
 * Ring Accelerator
 *
 * Ring accelerator SoC-specific configuration definitions
 *
 * Data version: 230209_212853
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
#ifndef RA_CFG_H
#define RA_CFG_H

/**
 * DMASS0_RINGACC_0 total number of rings before secure proxy rings
 */
#define RA_DMASS0_RINGACC_0_MAX_INDEX_PRE_SP (0U)
/**
 * DMASS0_RINGACC_0 total number of rings after secure proxy rings
 */
#define RA_DMASS0_RINGACC_0_MAX_INDEX_POST_SP (12U)
/**
 * Ring accelerator ring event index for rings before secure proxy rings.
 */
#define RA_DMASS0_RINGACC_0_RING_OES_EVT_INDEX_OFFSET_PRE_SP (0x14U)
/**
 * Ring accelerator ring event index for rings after secure proxy rings.
 */
#define RA_DMASS0_RINGACC_0_RING_OES_EVT_INDEX_OFFSET_POST_SP (20U)
/**
 * Ring accelerator global control error event index.  Must be greater than
 * the max of all SoC RAs max ring index to avoid event indexing conflicts.
 */
#define RA_GCFG_ERROR_EVT_INDEX (0x20U)
/**
 * Total number of RA instances
 */
#define RA_INST_RANGE (3U)

#endif /* RA_CFG_H */
