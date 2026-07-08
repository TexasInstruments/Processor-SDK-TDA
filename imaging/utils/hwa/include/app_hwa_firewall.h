/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file app_hwa_firewall.h
 *
 * \brief Common utilities and functions for HWA SL2 firewall management
 *
 * This file provides common functionality for hardware accelerators (VPAC/DMPAC) including:
 * - SL2 Firewall configuration for secure memory protection
 * - Reference-counted firewall management for multi-handle scenarios
 * - Shared constants and definitions
 *
 * Supported accelerators: VPAC IPs (MSC, LDC, VISS, NF), DMPAC IPs (DOF, SDE)
 */

#ifndef APP_HWA_FIREWALL_H_
#define APP_HWA_FIREWALL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#ifndef VPAC3L

/* SL2 Firewall Enable/Disable Flags */
#define APP_SL2_FIREWALL_DISABLE    (0U)
#define APP_SL2_FIREWALL_ENABLE     (1U)

/* CPU Privilege IDs */
#define TISCI_PRIV_ID_MAIN_0_R5_0   (212U)  /* VPAC R5 core */
#define TISCI_PRIV_ID_MAIN_0_R5_1   (213U)  /* DMPAC R5 core */
#define TISCI_PRIV_ID_MAIN_2_R5_0   (216U)  /* VPAC1 R5 core (J784S4) */

/* VPAC Accelerator Privilege IDs */
#define TISCI_PRIV_ID_VISS          (213U)  /* VISS accelerator */
#define TISCI_PRIV_ID_LDC           (215U)  /* LDC accelerator */
#define TISCI_PRIV_ID_MSC           (217U)  /* MSC accelerator */
#define TISCI_PRIV_ID_NF            (218U)  /* NF accelerator */

/* DMPAC Accelerator Privilege IDs */
#define TISCI_PRIV_ID_SDE           (208U)  /* SDE accelerator */
#define TISCI_PRIV_ID_DOF_RD        (209U)  /* DOF read master */
#define TISCI_PRIV_ID_DOF           (210U)  /* DOF main master */
#define TISCI_PRIV_ID_FOCO_0        (211U)  /* FOCO module 0 (used by DOF) */
#define TISCI_PRIV_ID_FOCO_1        (212U)  /* FOCO module 1 (used by SDE) */

/* SL2 Memory Banks Configuration */
#define VHWA_NUM_SL2_BANKS          (4U)    /* Number of SL2 memory banks per accelerator */

/* ============================================================================
 * Firewall Configuration
 * ============================================================================
 *
 * The firewall is configured once during system initialization and remains
 * active for the lifetime of the application.
 *
 * USAGE:
 *
 * Firewall is automatically configured via remote service during app_init.c:
 *   - VPAC firewall: appVhwaVpacRemoteServiceInit() -> APP_VPAC_ENABLE_FIREWALL
 *   - DMPAC firewall: appVhwaRemoteServiceInit() -> APP_DMPAC_ENABLE_FIREWALL
 *
 * Once enabled, the firewall protects SL2 memory throughout application runtime.
 *
 * ============================================================================
 */

/* VPAC Firewall Region Assignments */
#define VPAC_FWL_REGION_LDC         (0U)  /* LDC accelerator */
#define VPAC_FWL_REGION_MSC         (1U)  /* MSC accelerator */
#define VPAC_FWL_REGION_NF          (2U)  /* NF accelerator */
#define VPAC_FWL_REGION_VISS        (3U)  /* VISS accelerator */

/* DMPAC Firewall Region Assignments (3 regions total for DMPAC) */
#define DMPAC_FWL_REGION_DOF_MAIN       (0U)  /* Region 0: DOF Main - Current+FOCO_cur+Predictors+Output */
#define DMPAC_FWL_REGION_DOF_REFERENCE  (1U)  /* Region 1: DOF Reference - Reference+FOCO_ref */
#define DMPAC_FWL_REGION_SDE            (2U)  /* Region 2: SDE - All SDE buffers */

/* Region Index Aliases for clarity */
#define DMPAC_FWL_REGION_0              DMPAC_FWL_REGION_DOF_MAIN       /* Region 0 */
#define DMPAC_FWL_REGION_1              DMPAC_FWL_REGION_DOF_REFERENCE  /* Region 1 */
#define DMPAC_FWL_REGION_2              DMPAC_FWL_REGION_SDE            /* Region 2 */

#endif /* VPAC3L */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#ifndef VPAC3L
/**
 * \brief Configure SL2 firewall for VPAC
 *
 * Configures hardware firewall to protect VPAC SL2 memory region.
 * Called once during system initialization via remote service.
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 * \param regionIdx  Firewall region index (0-3 for LDC/MSC/NF/VISS)
 * \param startAddr  Start address (MUST be 16KB aligned)
 * \param size       Size in bytes (MUST be 16KB aligned)
 * \param ipPrivId   IP Accelerator Privilege ID
 * \param r5PrivId   R5 Core Privilege ID (overridden on J784S4 based on vpacInstId)
 * \param focoPrivId FOCO Privilege ID (0 if not used)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
vx_status appVpacFirewallEnable(
    uint8_t vpacInstId,
    uint8_t regionIdx,
    uint64_t startAddr,
    uint32_t size,
    uint8_t ipPrivId,
    uint8_t r5PrivId,
    uint8_t focoPrivId);

/**
 * \brief Configure SL2 firewall for DMPAC
 *
 * Configures hardware firewall to protect DMPAC SL2 memory region.
 * Called once during system initialization via remote service.
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0, 1 for DMPAC1 on J784S4)
 * \param regionIdx  Firewall region index (0-2 for DOF_MAIN/DOF_REF/SDE)
 * \param startAddr  Start address (MUST be 16KB aligned)
 * \param size       Size in bytes (MUST be 16KB aligned)
 * \param ipPrivId   IP Accelerator Privilege ID
 * \param r5PrivId   R5 Core Privilege ID (overridden on J784S4 based on dmpacInstId)
 * \param focoPrivId FOCO Privilege ID (0 if not used)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
vx_status appDmpacFirewallEnable(
    uint8_t dmpacInstId,
    uint8_t regionIdx,
    uint64_t startAddr,
    uint32_t size,
    uint8_t ipPrivId,
    uint8_t r5PrivId,
    uint8_t focoPrivId);

/**
 * \brief Disable SL2 firewall for VPAC
 *
 * Disables hardware firewall for VPAC SL2 memory region.
 * Called during cleanup or when firewall protection is no longer needed.
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 * \param regionIdx  Firewall region index (0-3 for LDC/MSC/NF/VISS)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
vx_status appVpacFirewallDisable(
    uint8_t vpacInstId,
    uint8_t regionIdx);

/**
 * \brief Disable SL2 firewall for DMPAC
 *
 * Disables hardware firewall for DMPAC SL2 memory region.
 * Called during cleanup or when firewall protection is no longer needed.
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0, 1 for DMPAC1 on J784S4)
 * \param regionIdx   Firewall region index (0-2 for DOF_MAIN/DOF_REF/SDE)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
vx_status appDmpacFirewallDisable(
    uint8_t dmpacInstId,
    uint8_t regionIdx);

/**
 * \brief Disable all VPAC SL2 firewall regions for BIST
 *
 * Disables all SL2 firewall regions for a specific VPAC instance before BIST.
 * BIST (Built-In Self Test for LBIST or PBIST) requires unrestricted SL2 access and
 * performs hardware reset which clears firewall configuration.
 *
 * CRITICAL: This function disables hardware firewalls but PRESERVES state flags.
 * The flags remain set to indicate which firewalls should be re-enabled after
 * BIST, ensuring correct state restoration.
 *
 * Disables enabled regions for the specified VPAC: LDC, MSC, NF, VISS
 *
 * BIST Usage Flow:
 * 1. Call this function before BIST - disables HW, preserves flags
 * 2. Run BIST (hardware powers down/resets)
 * 3. Use appVpacFirewallShouldEnable() to query which firewalls to restore
 * 4. Re-enable only firewalls where flag = 1
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 *
 * \return VX_SUCCESS if all enabled VPAC firewalls disabled successfully
 *         VX_FAILURE if any firewall disable operation fails
 *
 * \note State flags preserved to track which firewalls should be restored
 */
vx_status appVpacFirewallDisableAll(uint8_t vpacInstId);

/**
 * \brief Disable all DMPAC SL2 firewall regions for BIST
 *
 * Disables all SL2 firewall regions for DMPAC before BIST.
 * BIST (Built-In Self Test for LBIST or PBIST) requires unrestricted SL2 access and
 * performs hardware reset which clears firewall configuration.
 *
 * CRITICAL: This function disables hardware firewalls but PRESERVES state flags.
 * The flags remain set to indicate which firewalls should be re-enabled after
 * BIST, ensuring correct state restoration.
 *
 * Disables enabled DMPAC regions: DOF_MAIN, DOF_REFERENCE, SDE
 *
 * BIST Usage Flow:
 * 1. Call this function before BIST - disables HW, preserves flags
 * 2. Run BIST (hardware powers down/resets)
 * 3. Use appDmpacFirewallShouldEnable() to query which firewalls to restore
 * 4. Re-enable only firewalls where flag = 1
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0, only 1 DMPAC exists)
 *
 * \return VX_SUCCESS if all enabled DMPAC firewalls disabled successfully
 *         VX_FAILURE if any firewall disable operation fails
 *
 * \note State flags preserved to track which firewalls should be restored
 */
vx_status appDmpacFirewallDisableAll(uint8_t dmpacInstId);

/**
 * \brief Query if VPAC firewall region should be enabled after BIST
 *
 * Checks the state tracking to determine if a VPAC firewall region was
 * enabled before BIST and should be re-enabled during reconfiguration.
 *
 * Use this in appVhwaVpacRemoteServiceInit() to conditionally enable firewalls:
 * - If returns 1: Enable firewall (was enabled before BIST)
 * - If returns 0: Skip firewall enable (was disabled before BIST)
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 * \param regionIdx  Firewall region index (0-3 for LDC/MSC/NF/VISS)
 *
 * \return 1 if firewall should be enabled (was enabled before BIST)
 *         0 if firewall should NOT be enabled (was disabled before BIST)
 */
uint8_t appVpacFirewallShouldEnable(uint8_t vpacInstId, uint8_t regionIdx);

/**
 * \brief Query if DMPAC firewall region should be enabled after BIST
 *
 * Checks the state tracking to determine if a DMPAC firewall region was
 * enabled before BIST and should be re-enabled during reconfiguration.
 *
 * Use this in appVhwaRemoteServiceInit() to conditionally enable firewalls:
 * - If returns 1: Enable firewall (was enabled before BIST)
 * - If returns 0: Skip firewall enable (was disabled before BIST)
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0, only 1 DMPAC exists)
 * \param regionIdx   Firewall region index (0-2 for DOF_MAIN/DOF_REF/SDE)
 *
 * \return 1 if firewall should be enabled (was enabled before BIST)
 *         0 if firewall should NOT be enabled (was disabled before BIST)
 */
uint8_t appDmpacFirewallShouldEnable(uint8_t dmpacInstId, uint8_t regionIdx);

#endif /* VPAC3L */

#ifdef __cplusplus
}
#endif

#endif /* APP_HWA_FIREWALL_H_ */
