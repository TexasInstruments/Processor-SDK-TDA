/*
 * RM
 *
 * Resource Manager APIs
 *
 * Copyright (C) 2017-2026, Texas Instruments Incorporated
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

#ifndef RM_H
#define RM_H

#include <config.h>
#include <types/short_types.h>
#include <types/address_types.h>
#include <types/errno.h>
#include <boardcfg/boardcfg_rm_data.h>
#include <soc_devgrps.h>

#ifdef CONFIG_RM
/**
 * \brief RM init function
 *
 * Initializes RM data structures and calls individual resource subsystem
 * initialization routines.
 *
 * \return
 * Status of initialization
 *      SUCCESS - Initialization completed successfully
 */
s32 rm_init(void);

/**
 * \brief RM de-init function
 *
 * De-initializes RM data structures in a device group
 *
 * \param devgrp Device group for which deinit has to be done
 * \param rom_deinit Boolean value to decide whether to reset clearing of
 *						ROM usage flags or not
 * \return
 * Status of de-initialization
 *      SUCCESS - De-initialization completed successfully
 */
s32 rm_deinit(devgrp_t devgrp, sbool rom_deinit);

/**
 * \brief Resource range retrieval
 *
 * \param msg_recv Received TISCI message
 *
 * \param msg_resp Response TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_core_get_resource_range(u32 *msg_recv, u32 *msg_resp);
#else
static inline s32 rm_init(void)
{
	return SUCCESS;
}

static inline s32 rm_deinit(devgrp_t devgrp __attribute__((unused)), sbool rom_deinit __attribute__((unused)))
{
	return SUCCESS;
}

static inline s32 rm_core_get_resource_range(u32	*msg_recv __attribute__((unused)),
					     u32	*msg_resp __attribute__((unused)))
{
	return -EINIT;
}
#endif

#ifdef CONFIG_RM_IRQ
/**
 * \brief IRQ set message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_irq_set(u32 *msg_recv);

/**
 * \brief IRQ release message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_irq_release(u32 *msg_recv);
#else
static inline s32 rm_irq_set(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}

static inline s32 rm_irq_release(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
#endif

#ifdef CONFIG_RM_PROXY
/**
 * \brief proxy configure message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_proxy_cfg(u32 *msg_recv);
#else
static inline s32 rm_proxy_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
#endif

#ifdef CONFIG_RM_RA
/**
 * \brief Ring accelerator configure message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_ra_cfg(u32 *msg_recv);

/**
 * \brief Ring accelerator monitor configure message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_ra_mon_cfg(u32 *msg_recv);
#else
static inline s32 rm_ra_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_ra_mon_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
#endif

#ifdef CONFIG_RM_UDMAP
/**
 * \brief UDMAP global configuration configure message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_gcfg_cfg(u32 *msg_recv);

/**
 * \brief UDMAP transmit channel configure message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_tx_ch_cfg(u32 *msg_recv);

#ifdef CONFIG_UDMAP_MULTI_CH_CFG
/**
 * \brief UDMAP transmit multiple channel configure message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_tx_multi_ch_cfg(u32 *msg_recv);
#else
static inline s32 rm_udmap_tx_multi_ch_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
#endif

/**
 * \brief UDMAP receive channel configure message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_rx_ch_cfg(u32 *msg_recv);

/**
 * \brief UDMAP receive flow standard configuration message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_flow_cfg(u32 *msg_recv);

/**
 * \brief UDMAP receive flow size threshold based free queue routing config
 *        message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_flow_size_thresh_cfg(u32 *msg_recv);

/**
 * \brief DMA flow delegate message handler
 *
 * \param msg_recv TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_udmap_flow_delegate(u32 *msg_recv);
#else
static inline s32 rm_udmap_gcfg_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_udmap_tx_ch_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_udmap_rx_ch_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_udmap_flow_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_udmap_flow_size_thresh_cfg(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_udmap_flow_delegate(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
#endif

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
#ifdef CONFIG_RM_PSIL
/**
 * \brief psil pairing message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_psil_pair(u32 *msg_recv);

/**
 * \brief psil unpairing message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_psil_unpair(u32 *msg_recv);

/**
 * \brief psil reading message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \param msg_resp Response TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_psil_read(u32 *msg_recv, u32 *msg_resp);

/**
 * \brief psil writing message handler
 *
 * \param msg_recv Received TISCI message
 *
 * \return SUCCESS if message processed successfully, else error
 */
s32 rm_psil_write(u32 *msg_recv);
#else
static inline s32 rm_psil_pair(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_psil_unpair(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_psil_read(u32	*msg_recv __attribute__((unused)),
			       u32	*msg_resp __attribute__((unused)))
{
	return -EINIT;
}
static inline s32 rm_psil_write(u32 *msg_recv __attribute__((unused)))
{
	return -EINIT;
}
#endif
#endif

#endif /* RM_H */
