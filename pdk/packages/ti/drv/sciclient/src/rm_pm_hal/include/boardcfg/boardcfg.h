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
 * \brief Receive a boardcfg security placed at an external address and validate
 *
 * \param host Host ID of the sender
 * \param boardcfg_securityp_low Low 32-bit of boardcfg security struct address.
 * \param boardcfg_securityp_high High 32-bit of boardcfg security struct address.
 * \param boardcfg_security_size Size of the full boardcfg security struct.
 * \param boardcfg_security_devgrp Device group for the security configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_security_receive_and_validate(u8 host, u32 boardcfg_securityp_low, u32 boardcfg_securityp_high, u16 boardcfg_security_size, devgrp_t boardcfg_security_devgrp);

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
 * \brief Copy the boardcfg hashes into dedicated memory.
 *
 * On a HS device, boardcfg needs to be integrity checked using hashes. In a
 * boot time optimized approach, these hashes are received via the SYSFW Outer
 * certificate. The SYSFW outer certificate is in a memory location used for
 * other purposes by SYSFW. This function copies the provided board
 * configuration hashes to a different location.
 *
 * This function is expected to be called during SYSFW init before sending the
 * ready notification to R5. This function does not parse the X509 certificate.
 * It only copies the provided hashes.
 *
 * \param sec_bcfg_hash pointer to security boardcfg hash
 * \param sec_bcfg_hash_len Length of the security boardcfg hash
 * \param sec_bcfg_iv pointer to security boardcfg iv
 * \param sec_bcfg_iv_len Length of the security boardcfg iv
 * \param sec_bcfg_rs pointer to security boardcfg rs
 * \param sec_bcfg_rs_len Length of the security boardcfg rs
 * \param pm_bcfg_hash pointer to pm boardcfg hash
 * \param pm_bcfg_hash_len Length of the pm boardcfg hash
 * \param rm_bcfg_hash pointer to rm boardcfg hash
 * \param rm_bcfg_hash_len Length of the rm boardcfg hash
 * \param core_bcfg_hash pointer to core boardcfg hash
 * \param core_bcfg_hash_len Length of the core boardcfg hash
 * \param sec_bcfg_ver security boardcfg version for roll back protection
 * \param sec_bcfg_num_iter Number of iterations for the KDF
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_receive_bcfg_hashes(const u8 *sec_bcfg_hash, u32 sec_bcfg_hash_len, const u8 *sec_bcfg_iv, u32 sec_bcfg_iv_len, const u8 *sec_bcfg_rs, u32 sec_bcfg_rs_len, const u8 *pm_bcfg_hash,
				 u32 pm_bcfg_hash_len, const u8 *rm_bcfg_hash, u32 rm_bcfg_hash_len, const u8 *core_bcfg_hash, u32 core_bcfg_hash_len, u8 sec_bcfg_ver,
				 u8 sec_bcfg_num_iter);

/**
 * \brief return the attributes for the specified otp mmr from board configuration
 *
 * \param mmr_idx Index of the MMR
 * \param owner   on return, contains the host id of the owner of this MMR
 * \param attr    on return, contains the attributes of this MMR
 *
 * \return Error code on failure, SUCCESS otherwise.
 *
 */
s32 boardcfg_sec_get_otp_mmr_attr(u8 mmr_idx, u8 *owner, u8 *attr);

/**
 * \brief check if the host has write permission to extended OTP area
 *
 * \param host Host ID of the sender
 *
 * \return STRUE if host has write permissions, SFALSE if board configuration
 * has not been received or if the host does not have write permissions
 */
sbool boardcfg_sec_is_write_host(u8 host);

/**
 * \brief check if the host can use DKEK
 *
 * \param host Host ID of the sender
 *
 * \return STRUE if host can use DKEK, SFALSE if board configuration
 * has not been received or if the host does not have permission
 */
sbool boardcfg_sec_is_dkek_accessible(u8 host);

/**
 * \brief Check if the host may access SA2UL authentication resources owned by
 *        SYSFW
 *
 * \param host Host ID of the host allowed to request acquire/release of the
 *             resources
 *
 * \return STRUE if host has permissions to make acquire/release request, SFALSE
 *         if board configuration has not been received or if the host does not
 *         have the access permissions.
 */
sbool boardcfg_sec_sa2ul_auth_res_accessible(u8 host);

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
 * \brief Process a boardcfg security structure placed at a specific address
 *
 * \param host Host ID of the sender
 * \param boardcfg_securityp_low Low 32-bit of boardcfg security struct address.
 * \param boardcfg_securityp_high High 32-bit of boardcfg security struct address.
 * \param boardcfg_security_size Size of the full boardcfg security struct.
 * \param boardcfg_security_devgrp Device group for the Security configuration.
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_sec_process(u8 host, u32 boardcfg_securityp_low, u32 boardcfg_securityp_high, u16 boardcfg_security_size, devgrp_t boardcfg_security_devgrp);

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
/**
 * \brief check if JTAG can be unlocked at runtime using a signed certificate
 *
 * \return STRUE if jtag unlock is allowed, SFALSE if it is not.
 */
sbool boardcfg_sec_is_jtag_unlock_allowed(void);

/**
 * \brief check if the certificate revision in the jtag unlock certificate is
 *        above the specified minimum. This is used for anti rollback protection.
 *
 * \param swrv Revision specified in the certificate.
 *
 * \return STRUE if swrv is above the specified minimum, SFALSE otherwise.
 */
sbool boardcfg_sec_check_jtag_unlock_cert_rev(u32 swrv);

/**
 * \brief check if the specified host can do jtag unlock with a certificate
 *
 * \param host_id ID of host sending the jtag unlock command
 *
 * \return STRUE if the host can unlock jtag with a certificate, SFALSE otherwise.
 */
sbool boardcfg_sec_check_jtag_unlock_host(u8 host_id);

/**
 * \brief check if JTAG can be unlocked at runtime using a wildcard signed certificate
 *
 * \return STRUE if wildcard jtag unlock is allowed, SFALSE if it is not.
 */
sbool boardcfg_sec_is_wildcard_jtag_unlock_allowed(void);

/**
 * \brief Get the security device group from the local boardcfg_security data.
 *
 * \param devgrp Pointer to a device group variable in which the device group
 *        is returned on SUCCESS
 *
 * \return EINIT if the local security board data has not yet been received or is
 *         invalid, SUCCESS if in the board data has been received and is valid.
 */
s32 boardcfg_get_sec_devgrp(devgrp_t *devgrp);

/**
 * \brief Trigger automatic boardcfg processing using the given handler
 *
 * \param msg_sender ID of the host sending the handover message
 * \param new_owner ID of the host receiving the security handover
 *
 * \return Error code on failure, SUCCESS otherwise.
 */
s32 boardcfg_sec_get_handover_hosts(u8 *msg_sender, u8 *new_owner);

/**
 * \brief Set the security device group from the local boardcfg_security data.
 *
 * \param devgrp Device group variable
 *
 * \return EINIT when the devgrp is not set
 *         SUCCESS if the devgrp is set
 */
s32 boardcfg_set_sec_devgrp(devgrp_t boardcfg_security_devgrp);

#endif /* BOARD_CFG_API_H */
