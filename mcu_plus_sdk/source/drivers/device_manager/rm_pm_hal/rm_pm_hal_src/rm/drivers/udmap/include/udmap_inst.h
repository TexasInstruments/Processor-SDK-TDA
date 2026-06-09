/*
 * UDMAP
 *
 * UDMAP driver instance definition
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
#ifndef UDMAP_INST_H
#define UDMAP_INST_H

#include <types/short_types.h>
#include <types/sbool.h>
#include <types/devgrps.h>
#include <soc/rm_mmr.h>
#include <rm_irq.h>
#include <udmap_cfg.h>

/**
 * UDMAP global configuration region
 */
#define UDMAP_GCFG                 (0U)
/**
 * BCDMA global configuration region
 */
#define BCDMA_GCFG                 (1U)
/**
 * PKTDMA global configuration region
 */
#define PKTDMA_GCFG                (2U)
/**
 * Total number of global configuration region types
 */
#define UDMAP_NUM_GCFG_TYPES       (3U)

/**
 * UDMAP transmit standard channel
 */
#define UDMAP_TX_CHAN              (0U)
/**
 * UDMAP transmit high capacity channel
 */
#define UDMAP_TX_HCHAN             (1U)
/**
 * UDMAP transmit ultra high capacity channel
 */
#define UDMAP_TX_UHCHAN            (2U)
/**
 * UDMAP transmit external channel
 */
#define UDMAP_TX_ECHAN             (3U)
/**
 * UDMAP receive standard channel
 */
#define UDMAP_RX_CHAN              (4U)
/**
 * UDMAP receive high capacity channel
 */
#define UDMAP_RX_HCHAN             (5U)
/**
 * UDMAP receive ultra high capacity channel
 */
#define UDMAP_RX_UHCHAN            (6U)
/**
 * DMSS BCDMA transmit channel
 */
#define DMSS_BCDMA_TX_CHAN         (7U)
/**
 * DMSS BCDMA receive channel
 */
#define DMSS_BCDMA_RX_CHAN         (8U)
/**
 * DMSS BCDMA block copy channel
 */
#define DMSS_BCDMA_BLOCK_COPY_CHAN (9U)
/**
 * DMSS PKTDMA transmit channel
 */
#define DMSS_PKTDMA_TX_CHAN        (10U)
/**
 * DMSS PKTDMA receive channel
 */
#define DMSS_PKTDMA_RX_CHAN        (11U)
/**
 * DMSS BCDMA block copy high capacity channel
 */
#define DMSS_BCDMA_BLOCK_COPY_HCCHAN (12U)
/**
 * DMSS BCDMA high capacity transmit channel
 */
#define DMSS_BCDMA_TX_HCCHAN             (13U)
/**
 * DMSS BCDMA high capacity receive channel
 */
#define DMSS_BCDMA_RX_HCCHAN             (14U)
/**
 * Total number of ring types
 */
#define UDMAP_NUM_CHAN_TYPES       (15U)

/**
 * \brief
 * UDMAP channel type information
 *
 * \param start
 * UDMAP channel type start index
 *
 * \param end
 * UDMAP channel type end index
 *
 * \param flow_start
 * UDMAP channel type mapped flows start index
 *
 * \param flow_end
 * UDMAP channel type mapped flows end index
 *
 * \param flow_utype
 * UDMAP flow type resource assignment type
 *
 * \param oes_offset
 * Event index offset for UDMAP channel's OES register
 *
 * \param err_oes_offset
 * Event index offset for UDMAP channel's error OES register
 *
 * \param type
 * UDMAP channel type
 *
 * \param utype
 * UDMAP channel type resource assignment type
 */
struct udmap_ch_type {
	const u16	start;
	const u16	end;
	const u16	flow_start;
	const u16	flow_end;
	const u16	flow_utype;
	const u16	oes_offset;
	const u16	err_oes_offset;
	const u8	type;
	const u16	utype;
};

/**
 * \brief
 * UDMAP shared flow type information
 *
 * \param start
 * UDMAP shared flow type start index
 *
 * \param end
 * UDMAP shared flow type end index
 *
 * \param utype
 * UDMAP shared flow type resource assignment type
 *
 * \param delegates
 * Shared flow delegate host array
 */
struct udmap_flow_common {
	const u16	start;
	const u16	end;
	const u16	utype;
	u8 *const	delegates;
};

/**
 * \brief
 * Describes the UDMA channels used by ROM and if they've been cleared back to
 * hardware reset values
 *
 * \param ch
 * channel used by ROM
 *
 * \param tx_ch
 * STRUE if transmit channel.  SFALSE if receive channel.
 *
 * \param err_evt
 * STRUE if channel's EOES.  SFALSE if channel's OES.
 *
 * \param cleared
 * Boolean storing whether or not the channel has been cleared
 */
struct udmap_used_mapping {
	const u16	ch;
	const sbool	tx_ch;
	const sbool	err_evt;
	sbool		cleared;
};

/**
 * \brief
 * UDMAP instance containing all data required to manage a Navigator Subsystem
 * UDMAP
 *
 * \param id
 * Device ID of NavSS UDMAP
 *
 * \param root_id
 * Device ID of NavSS in which UDMAP is located
 *
 * \param initialized
 * Instance initialization flag
 *
 * \param devgrp
 * Device group to which instance belongs
 *
 * \param gcfg
 * UDMAP subsystem's global configuration MMR region
 *
 * \param tchan
 * UDMAP subsystem's transmit channel configuration MMR region
 *
 * \param rchan
 * UDMAP subsystem's receive channel configuration MMR region
 *
 * \param bchan
 * BCDMA subsystem's block copy channel configuration MMR region
 *
 * \param rflow
 * UDMAP subsystem's receive flow configuration MMR region
 *
 * \param tchanrt
 * UDMAP subsystem's transmit channel real-time configuration MMR region
 *
 * \param rchanrt
 * UDMAP subsystem's receive channel real-time configuration MMR region
 *
 * \param bchanrt
 * BCDMA subsystem's block copy channel real-time configuration MMR region
 *
 * \param n_tx_ch
 * Total number of UDMAP transmit channels in the subsystem
 *
 * \param n_rx_ch
 * Total number of UDMAP receive channels in the subsystem
 *
 * \param n_bc_ch
 * Total number of BCDMA block copy channels in the subsystem
 *
 * \param n_rx_flow
 * Total number of UDMAP receive flows in the subsystem
 *
 * \param bc_ch_offset
 * API index offset to BCDMA block copy channel types
 *
 * \param tx_ch_types
 * Pointer to array of UDMAP transmit channel types
 *
 * \param rx_ch_types
 * Pointer to array of UDMAP receive channel types
 *
 * \param bc_ch_types
 * Pointer to array of BCDMA block copy channel types
 *
 * \param common_flow
 * Pointer to UDMAP shared flow details
 *
 * \param n_tx_ch_type
 * Number of UDMAP transmit channel types
 *
 * \param n_rx_ch_type
 * Number of UDMAP receive channel types
 *
 * \param n_bc_ch_type
 * Number of BCDMA block copy channel types
 *
 * \param invalid_flow_oes_offset
 * Event index mapped to UDMAP instance's invalid flow event
 *
 * \param invalid_flow_oes_utype
 * Resource assignment type mapped to UDMAP instance's invalid flow event
 *
 * \param gcfg_type
 * Type of GCFG region within the DMA instance.  Different DMAs support
 * different register sets within their GCFG region
 *
 * \param gcfg_utype
 * Resource assignment type mapped to UDMAP instance's global configuration
 * read and write access
 *
 * \param oes_handler
 * IRQ event source handler
 *
 * \param rom_usage
 * Channels used by ROM during boot that need their OES registers reset to
 * hardware reset values
 *
 * \param n_rom_usage
 * Number of entries in the rom_usage array
 *
 * \param dru_ch0_dst_thread_offset
 * Thread offset for DRU channel zero.  Needed for UDMA UTC configuration.
 */
struct udmap_instance {
	const u16				id;
	const u16				root_id;
	sbool					initialized;
	const devgrp_t				devgrp;
	const struct rm_mmr *const		gcfg;
	const struct rm_mmr *const		tchan;
	const struct rm_mmr *const		rchan;
	const struct rm_mmr *const		bchan;
	const struct rm_mmr *const		rflow;
	const struct rm_mmr *const		tchanrt;
	const struct rm_mmr *const		rchanrt;
	const struct rm_mmr *const		bchanrt;
	const u16				n_tx_ch;
	const u16				n_rx_ch;
	const u16				n_bc_ch;
	const u16				n_rx_flow;
	const u16				bc_ch_offset;
	const struct udmap_ch_type *const	tx_ch_types;
	const struct udmap_ch_type *const	rx_ch_types;
	const struct udmap_ch_type *const	bc_ch_types;
	struct udmap_flow_common *const		common_flow;
	const u8				n_tx_ch_type;
	const u8				n_rx_ch_type;
	const u8				n_bc_ch_type;
	const u16				invalid_flow_oes_offset;
	const u16				invalid_flow_oes_utype;
	const u8				gcfg_type;
	const u16				gcfg_utype;
	struct rm_irq_oes_src			oes_handler;
	struct udmap_used_mapping *const	rom_usage;
	const u8				n_rom_usage;
	const u16				dru_ch0_dst_thread_offset;
};

/**
 * Extern for UDMAP subsystem instances array
 */
extern struct udmap_instance udmap_inst[UDMAP_INST_RANGE];

/**
 * Extern for number of UDMAP subsystem instances
 */
extern const u32 udmap_inst_count;


#endif /* UDMAP_INST_H */
