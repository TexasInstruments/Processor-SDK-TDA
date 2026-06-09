/*
 * System Firmware
 *
 * TISCI Data structures for core messages
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

#ifndef MESSAGES_CORE_H
#define MESSAGES_CORE_H

#include <tisci/tisci_protocol.h>
#include <types/short_types.h>
#include <types/devgrps.h>

/** Bit fields of TISCI_MSG_QUERY_FW_CAPS message */
#define TISCI_MSG_FLAG_FW_CAP_GENERIC_CAP               BIT(0)
#define TISCI_MSG_FLAG_FW_CAP_LPM_DEEP_SLEEP            BIT(1)
#define TISCI_MSG_FLAG_FW_CAP_LPM_MCU_ONLY              BIT(2)
#define TISCI_MSG_FLAG_FW_CAP_LPM_STANDBY               BIT(3)
#define TISCI_MSG_FLAG_FW_CAP_LPM_PARTIAL_IO_ON         BIT(4)
#define TISCI_MSG_FLAG_FW_CAP_LPM_DM_MANAGED            BIT(5)
#define TISCI_MSG_FLAG_FW_CAP_LPM_IO_ONLY_PLUS_DDR      BIT(6)
#define TISCI_MSG_FLAG_FW_CAP_IO_ISOLATION              BIT(7)
#define TISCI_MSG_FLAG_FW_CAP_DM                        BIT(8)
#define TISCI_MSG_FLAG_FW_CAP_LPM_ABORT                 BIT(9)
#define TISCI_MSG_FLAG_FW_CAP_CLOCK_SSC                 BIT(10)
#define TISCI_MSG_FLAG_FW_CAP_LPM_ENCRYPT_IMAGE         BIT(11)
#define TISCI_MSG_FLAG_FW_CAP_LPM_BOARDCFG_MANAGED      BIT(12)
#define TISCI_MSG_FLAG_FW_CAP_GET_DEVICE_MULTIPLE       BIT(13)
#define TISCI_MSG_FLAG_FW_CAP_LPM_IRQ_CONTEXT_LOST      BIT(14)
#define TISCI_MSG_FLAG_FW_CAP_LPM_CLK_CONTEXT_LOST      BIT(15)

/**
 * \brief Notification message to indicate the DMSC is available.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_msg_version_req {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_VERSION request to provide version info about
 *	  currently running firmware.
 * \param hdr TISCI header.
 * \param str Human readable version string.
 * \param version Version number of the firmware.
 * \param abi_major Major version number of ABI in use by firmware.
 * \param abi_minor Minor version number of ABI in use by firmware.
 * \param sub_version Sub-version number of the firmware.
 * \param patch_version Patch-version number of the firmware.
 */
struct tisci_msg_version_resp {
	struct tisci_header	hdr;
	char			str[32];
	u16			version;
	u8			abi_major;
	u8			abi_minor;
	u8			sub_version;
	u8			patch_version;
} __attribute__((__packed__));

/**
 * \brief Notification message to get DM version.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_msg_dm_version_req {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_DM_VERSION response struct to provide version
 *      info about currently running DM firmware.
 * \param hdr TISCI header.
 * \param version Version number of the firmware.
 * \param sub_version Sub-version number of the firmware.
 * \param patch_version Patch-version number of the firmware.
 * \param abi_major Major version number of ABI in use by firmware.
 * \param abi_minor Minor version number of ABI in use by firmware.
 * \param rm_pm_hal_version Human readable rm_pm_hal version string.
 * \param sciserver_version Human readable Sciserver version string.
 */
struct tisci_msg_dm_version_resp {
	struct tisci_header	hdr;
	u16			version;
	u8			sub_version;
	u8			patch_version;
	u8			abi_major;
	u8			abi_minor;
	char			rm_pm_hal_version[12];
	char			sciserver_version[26];
} __attribute__((__packed__));

/**
 * \brief Notification message to indicate the DMSC is available.
 *
 * \param hdr TISCI header
 * \param extboot_status Status of extended boot. Applicable only for combined image
 */
struct tisci_msg_boot_notification_req {
	struct tisci_header	hdr;
	u32			extboot_status;
} __attribute__((__packed__));

/**
 * \brief Response to notification message about firmware availability.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_msg_boot_notification_resp {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_BOARD_CONFIG_RM request to provide the location and
 *	  size of the boardcfg Resource Management configuration structure.
 *
 * \param hdr TISCI header
 * \param boardcfg_rmp_low Low 32-bits of physical pointer to boardcfg
 *                         RM configuration struct.
 * \param boardcfg_rmp_high High 32-bits of physical pointer to boardcfg
 *                          RM configuration struct.
 * \param boardcfg_rm_size Size of RM configuration data.
 * \param boardcfg_rm_devgrp Device group for the RM configuration.
 */
struct tisci_msg_board_config_rm_req {
	struct tisci_header	hdr;
	u32			boardcfg_rmp_low;
	u32			boardcfg_rmp_high;
	u16			boardcfg_rm_size;
	devgrp_t		boardcfg_rm_devgrp;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_RM.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_rm_resp {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_RESASG.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_resasg_resp {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_BOARD_CONFIG request to provide the location and
 *	  size of the boardcfg structure.
 *
 * \param hdr TISCI header
 * \param boardcfgp_low Low 32-bits of physical pointer to boardcfg struct.
 * \param boardcfgp_high High 32-bits of physical pointer to boardcfg struct.
 * \param boardcfg_size Size of configuration data.
 * \param boardcfg_devgrp Device group for the base board configuration.
 *
 */
struct tisci_msg_board_config_req {
	struct tisci_header	hdr;
	u32			boardcfgp_low;
	u32			boardcfgp_high;
	u16			boardcfg_size;
	devgrp_t		boardcfg_devgrp;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_resp {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_BOARD_CONFIG_SECURITY request to provide the location and
 *	  size of the boardcfg security configuration structure.
 *
 * \param hdr TISCI header
 * \param boardcfg_securityp_low Low 32-bits of physical pointer to boardcfg
 *                             security configuration struct.
 * \param boardcfg_securityp_high High 32-bits of physical pointer to boardcfg
 *                              security configuration struct.
 * \param boardcfg_security_size Size of security configuration data.
 * \param boardcfg_security_devgrp Device group for the security board configuration.
 *
 */
struct tisci_msg_board_config_security_req {
	struct tisci_header	hdr;
	u32			boardcfg_securityp_low;
	u32			boardcfg_securityp_high;
	u16			boardcfg_security_size;
	devgrp_t		boardcfg_security_devgrp;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_SECURITY.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_security_resp {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_BOARD_CONFIG_PM request to provide the location and
 *	  size of the boardcfg Power Management configuration structure.
 *
 * \param hdr TISCI header
 * \param boardcfg_pmp_low Low 32-bits of physical pointer to boardcfg
 *                             PM configuration struct.
 * \param boardcfg_pmp_high High 32-bits of physical pointer to boardcfg
 *                              PM configuration struct.
 * \param boardcfg_pm_size Size of PM configuration data.
 * \param boardcfg_pm_devgrp  Device group for the PM configuration
 *
 */
struct tisci_msg_board_config_pm_req {
	struct tisci_header	hdr;
	u32			boardcfg_pmp_low;
	u32			boardcfg_pmp_high;
	u16			boardcfg_pm_size;
	devgrp_t		boardcfg_pm_devgrp;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_BOARD_CONFIG_PM.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_board_config_pm_resp {
	struct tisci_header hdr;
} __attribute__((__packed__));


/**
 * \brief TISCI_MSG_QUERY_MSMC request to provide currently available MSMC
 * memory if present in the SoC
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_query_msmc_req {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_QUERY_MSMC request response providing currently available
 * MSMC memory if present in the SoC
 * \param hdr TISCI header.
 * \param msmc_start_low Low 32-bits of physical address start of MSMC memory
 * \param msmc_start_high High 32-bits of physical address start of MSMC
 *			  memory
 * \param msmc_end_low Low 32-bits of physical addressable address
 *			(byte aligned) of valid range of MSMC memory.
 * \param msmc_end_high High 32-bits of physical addressable address
 *			(byte aligned) of valid range of MSMC memory.
 */
struct tisci_query_msmc_resp {
	struct tisci_header	hdr;
	u32			msmc_start_low;
	u32			msmc_start_high;
	u32			msmc_end_low;
	u32			msmc_end_high;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_GET_TRACE_CONFIG request to get the debug trace config from
 * base board config
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_get_trace_config_req {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_GET_TRACE_CONFIG request response providing the debug config
 * from base board config
 *
 * \param hdr TISCI header.
 * \param trace_dst_enables enabled destination traces
 * \param trace_src_enables enabled source traces
 */
struct tisci_get_trace_config_resp {
	struct tisci_header	hdr;
	u16			trace_dst_enables;
	u16			trace_src_enables;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_QUERY_FW_CAPS request to provide the firmware/SOC capabilities
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header
 */
struct tisci_query_fw_caps_req {
	struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief TISCI_MSG_QUERY_FW_CAPS request response providing currently available
 * SOC/firmware capabilities
 *
 * \param hdr TISCI header.
 * \param fw_caps Each bit in fw_caps indicating one FW/SOC capability
 *
 */
struct tisci_query_fw_caps_resp {
	struct tisci_header	hdr;
	u64			fw_caps;
} __attribute__((__packed__));

#endif /* MESSAGES_CORE_H */
