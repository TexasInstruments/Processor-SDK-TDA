/*
 * System Firmware Source File
 *
 * Board configuration Public API
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

#ifndef BOARD_CFG_API_H
#define BOARD_CFG_API_H

#include <types/devgrps.h>
#include <types/sbool.h>

/**
 * \brief Get a pointer to the local copy of the boardcfg_control data.
 *
 * \return NULL if the local board data has not yet been received or is
 *         invalid, pointer otherwise.
 */
const struct boardcfg_control *boardcfg_get_control(void);

/**
 * \brief Get a pointer to the local copy of the boardcfg_secproxy data.
 *
 * \return NULL if the local board data has not yet been received or is
 *         invalid, pointer otherwise.
 */
const struct boardcfg_secproxy *boardcfg_get_secproxy(void);

/**
 * \brief Get a pointer to the local copy of the boardcfg_msmc data.
 *
 * \return NULL if the local board data has not yet been received or is
 *         invalid, pointer otherwise.
 */
const struct boardcfg_msmc *boardcfg_get_msmc(void);

/**
 * \brief Get a pointer to the local copy of boardcfg_proc_acl data
 *
  \return NULL if the local board data has not yet been received or is
 *	   invalid, pointer otherwise.
 */
const struct boardcfg_proc_acl *boardcfg_get_proc_acl(void);

/**
 * \brief Get a pointer to the local copy of boardcfg_host_hierarchy data
 *
 * \return NULL if the local board data has not yet been received or is
 *	   invalid, pointer otherwise.
 */
const struct boardcfg_host_hierarchy *boardcfg_get_host_hierarchy(void);

/**
 * \brief Get a pointer to the local copy of the boardcfg_dbg_cfg data.
 *
 * \return NULL if the local board data has not yet been received or is
 *         invalid, pointer otherwise.
 */
const struct boardcfg_dbg_cfg *boardcfg_get_dbg_cfg(void);

/**
 * \brief Get a pointer to the local copy of the boardcfg_pmic_cfg data.
 *
 * \return NULL if the local board data has not yet been received or is
 *         invalid, pointer otherwise.
 */
const struct boardcfg_pmic_cfg *boardcfg_get_pmic_cfg(void);

/**
 * \brief Prototype for handler function to process boardcfg structure placed at specified addresss
 *
 * \param host Host ID of the sender
 * \param boardcfgp_low Low 32-bit of boardcfg struct address.
 * \param boardcfgp_high High 32-bit of boardcfg struct address.
 * \param boardcfg_size Size of the full boardcfg struct.
 * \param boardcfg_devgrp Device group for this boardcfg configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
typedef s32 (*boardcfg_process_fxn)(u8 host, u32 boardcfgp_low, u32 boardcfgp_high, u16 boardcfg_size, devgrp_t boardcfg_devgrp);

/**
 * \brief Receive a boardcfg power management structure placed at an
 *        external address and validate
 *
 * \param host Host ID of the sender
 * \param boardcfg_pmp_low Low 32-bit of boardcfg pm struct address.
 * \param boardcfg_pmp_high High 32-bit of boardcfg pm struct address.
 * \param boardcfg_pm_size Size of the full boardcfg pm struct.
 * \param boardcfg_pm_devgrp Device group for the PM configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_pm_receive_and_validate(u8 host, u32 boardcfg_pmp_low, u32 boardcfg_pmp_high, u16 boardcfg_pm_size, devgrp_t boardcfg_pm_devgrp);

/**
 * \brief Get a pointer to lpm boardcfg structure placed at an external address
 *
 * \param boardcfg_pmp_low Low 32-bit of boardcfg pm struct address.
 * \param boardcfg_pmp_high High 32-bit of boardcfg pm struct address.
 * \param boardcfg_pm_size Size of the full boardcfg pm struct.
 *
 * \return Pointer to the structure representing the LPM boardcfg, NULL in case of failure
 */
struct boardcfg_pm_lpm_cfg *boardcfg_pm_extract_lpm_cfg(void);

/**
 * \brief Is the LPM config inside of PM boarcfg valid
 * \return boolean
 */
bool is_lpm_boardcfg_valid(void);

/**
 * \brief Receive a boardcfg resource management structure placed at an
 *        external address and validate
 *
 * \param host Host ID of the sender
 * \param boardcfg_rmp_low Low 32-bit of boardcfg rm struct address.
 * \param boardcfg_rmp_high High 32-bit of boardcfg rm struct address.
 * \param boardcfg_rm_size Size of the full boardcfg rm struct.
 * \param boardcfg_rm_devgrp Device group for the RM configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_rm_receive_and_validate(u8 host, u32 boardcfg_rmp_low, u32 boardcfg_rmp_high, u16 boardcfg_rm_size, devgrp_t boardcfg_rm_devgrp);

/**
 * \brief Receive a boardcfg placed at an external address and validate
 *
 * \param host Host ID of the sender
 * \param boardcfgp_low Low 32-bit of boardcfg struct address.
 * \param boardcfgp_high High 32-bit of boardcfg struct address.
 * \param boardcfg_size Size of the full boardcfg struct.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_receive_and_validate(u8 host, u32 boardcfgp_low, u32 boardcfgp_high, u16 boardcfg_size);

/**
 * \brief Process a boardcfg structure placed at a specific address
 *
 * \param host Host ID of the sender
 * \param boardcfgp_low Low 32-bit of boardcfg struct address.
 * \param boardcfgp_high High 32-bit of boardcfg struct address.
 * \param boardcfg_size Size of the full boardcfg struct.
 * \param boardcfg_devgrp Device group for this boardcfg configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_process(u8 host, u32 boardcfgp_low, u32 boardcfgp_high, u16 boardcfg_size, devgrp_t boardcfg_devgrp);

/**
 * \brief Process a boardcfg power management structure placed at a
 *        specific address
 *
 * \param host Host ID of the sender
 * \param boardcfg_pmp_low Low 32-bit of boardcfg pm struct address.
 * \param boardcfg_pmp_high High 32-bit of boardcfg pm struct address.
 * \param boardcfg_pm_size Size of the full boardcfg pm struct.
 * \param boardcfg_pm_devgrp Device group for the PM configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_pm_process(u8 host, u32 boardcfg_pmp_low, u32 boardcfg_pmp_high, u16 boardcfg_pm_size, devgrp_t boardcfg_pm_devgrp);

/**
 * \brief Process a boardcfg resource management structure placed at a
 *        specific address
 *
 * \param host Host ID of the sender
 * \param boardcfg_rmp_low Low 32-bit of boardcfg rm struct address.
 * \param boardcfg_rmp_high High 32-bit of boardcfg rm struct address.
 * \param boardcfg_rm_size Size of the full boardcfg rm struct.
 * \param boardcfg_rm_devgrp Device group for the RM configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_rm_process(u8 host, u32 boardcfg_rmp_low, u32 boardcfg_rmp_high, u16 boardcfg_rm_size, devgrp_t boardcfg_rm_devgrp);

/**
 * \brief Trigger automatic boardcfg processing using the given handler
 *
 * \param type Type of boardcfg
 * \param fxn Handler function for processing the boardcfg type
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_auto(u16 type, boardcfg_process_fxn fxn);

/**
 * \brief Trigger automatic boardcfg processing using the given handler
 *
 * \param type Type of boardcfg
 * \param fxn Handler function for processing the boardcfg type
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_user_auto(u16 type, boardcfg_process_fxn fxn);

#endif /* BOARD_CFG_API_H */
