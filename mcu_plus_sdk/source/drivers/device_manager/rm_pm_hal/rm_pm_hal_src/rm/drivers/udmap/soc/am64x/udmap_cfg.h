/*
 * UDMAP
 *
 * UDMAP SoC-specific configuration definitions
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
#ifndef UDMAP_CFG_H
#define UDMAP_CFG_H

/**
 * DMASS0_BCDMA_0 total number of UDMAP transmit channels
 */
#define UDMAP_DMASS0_BCDMA_0_TX_CHAN_MAX_INDEX (20U)
/**
 * DMASS0_BCDMA_0 total number of UDMAP receive channels
 */
#define UDMAP_DMASS0_BCDMA_0_RX_CHAN_MAX_INDEX (20U)
/**
 * DMASS0_BCDMA_0 total number of block copy channels
 */
#define UDMAP_DMASS0_BCDMA_0_BC_CHAN_MAX_INDEX (28U)
/**
 * DMASS0_BCDMA_0 block copy channel offset
 */
#define UDMAP_DMASS0_BCDMA_0_BC_CHAN_OFFSET (0U)
/**
 * DMASS0_PKTDMA_0 total number of UDMAP transmit channels
 */
#define UDMAP_DMASS0_PKTDMA_0_TX_CHAN_MAX_INDEX (42U)
/**
 * DMASS0_PKTDMA_0 total number of UDMAP receive channels
 */
#define UDMAP_DMASS0_PKTDMA_0_RX_CHAN_MAX_INDEX (29U)
/**
 * DMASS0_PKTDMA_0 total number of UDMAP receive flows
 */
#define UDMAP_DMASS0_PKTDMA_0_RX_FLOW_MAX_INDEX (192U)
/**
 * Total number of UDMA instances
 */
#define UDMAP_INST_RANGE (2U)

#endif /* UDMAP_CFG_H */
