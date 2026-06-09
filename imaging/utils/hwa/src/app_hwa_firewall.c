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
 * \file app_hwa_firewall.c
 *
 * \brief Common utilities implementation for HWA (VPAC/DMPAC) SL2 firewall management
 *
 * This file provides firewall configuration for VPAC and DMPAC SL2 memory regions.
 * Firewall is configured once during system initialization and remains active.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <VX/vx.h>
#include "utils/hwa/include/app_hwa_firewall.h"
#include "VX/vx_types.h"
#include <TI/tivx.h>
#include <inttypes.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#ifndef VPAC3L
/* VPAC SL2 Firewall Bank IDs (internal use only - not exposed in header) */
#define APP_VPAC_FWL_BANK0_ID    (6080U)
#define APP_VPAC_FWL_BANK1_ID    (6081U)
#define APP_VPAC_FWL_BANK2_ID    (6082U)
#define APP_VPAC_FWL_BANK3_ID    (6083U)

/* DMPAC SL2 Firewall Bank IDs (internal use only - not exposed in header) */
#define APP_DMPAC_FWL_BANK0_ID    (6016U)
#define APP_DMPAC_FWL_BANK1_ID    (6017U)
#define APP_DMPAC_FWL_BANK2_ID    (6018U)
#define APP_DMPAC_FWL_BANK3_ID    (6019U)
#endif /* VPAC3L */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

#ifndef VPAC3L

#include <ti/csl/csl_fvid2_dataTypes.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/sciclient/soc/sysfw/include/j721s2/tisci_hosts.h>
#include <ti/csl/soc.h>
#define FWL_TIMEOUT_VALUE    SCICLIENT_SERVICE_WAIT_FOREVER
#define VPAC_HOST_ID         TISCI_HOST_ID_MAIN_0_R5_0

/* ========================================================================== */
/*                         Internal Constants                                 */
/* ========================================================================== */

/* VPAC SL2 Firewall Bank IDs (common for all VPAC modules: VISS, LDC, MSC, NF) */
static const uint16_t VPAC_FWL_IDS[VHWA_NUM_SL2_BANKS] = {
    APP_VPAC_FWL_BANK0_ID,
    APP_VPAC_FWL_BANK1_ID,
    APP_VPAC_FWL_BANK2_ID,
    APP_VPAC_FWL_BANK3_ID
};

/* DMPAC SL2 Firewall Bank IDs (common for all DMPAC modules: DOF, SDE) */
static const uint16_t DMPAC_FWL_IDS[VHWA_NUM_SL2_BANKS] = {
    APP_DMPAC_FWL_BANK0_ID,
    APP_DMPAC_FWL_BANK1_ID,
    APP_DMPAC_FWL_BANK2_ID,
    APP_DMPAC_FWL_BANK3_ID
};

/* ========================================================================== */
/*                   FIREWALL STATE TRACKING FOR LBIST                        */
/* ========================================================================== */

/**
 * Firewall state tracking for LBIST support
 * Tracks which firewall regions are enabled before LBIST, so they can be
 * selectively restored after LBIST completion.
 */

/* Maximum VPAC instances: 1 for J721S2/J742S2, 2 for J784S4 */
#define MAX_VPAC_INSTANCES  (2U)
/* VPAC has 4 regions: LDC=0, MSC=1, NF=2, VISS=3 */
#define MAX_VPAC_REGIONS    (4U)
/* DMPAC has 3 regions: DOF_MAIN=0, DOF_REF=1, SDE=2 */
#define MAX_DMPAC_REGIONS   (3U)

/* Track if firewall regions are enabled for each VPAC instance
 * Default to 0 (disabled) - firewalls only enabled when customer explicitly calls enable.
 *
 * Normal flow (customer wants firewalls):
 *   - Call appVhwaEnableVpacFirewall() via APP_VPAC_ENABLE_FIREWALL command
 *   - Enables all firewalls unconditionally, sets flags to 1
 *
 * LBIST flow (after firewall enabled):
 *   - Call appVpacFirewallDisableAll() -> disables HW, preserves flags=1
 *   - Run LBIST (hardware resets)
 *   - Call appVhwaEnableVpacFirewallAfterLbist() -> checks flags, re-enables only modules where flag=1 */
static uint8_t gVpacFirewallEnabled[MAX_VPAC_INSTANCES][MAX_VPAC_REGIONS] = {
    {0, 0, 0, 0},  /* VPAC0: All disabled by default */
    {0, 0, 0, 0}   /* VPAC1: All disabled by default */
};

/* Track if firewall regions are enabled for DMPAC (only 1 instance)
 * Default to 0 (disabled) - firewalls only enabled when customer explicitly calls enable. */
static uint8_t gDmpacFirewallEnabled[MAX_DMPAC_REGIONS] = {
    0,  /* DOF_MAIN=0 */
    0,  /* DOF_REFERENCE=0 */
    0   /* SDE=0 */
};

/* ========================================================================== */
/*                   SL2 FIREWALL CONFIGURATION                               */
/* ========================================================================== */

/**
 * \brief Configure SL2 Firewall for VPAC/DMPAC
 *
 * Configures hardware firewall to protect accelerator memory region.
 * Called once during system initialization via remote service.
 *
 * \param fwlIds     Array of VHWA_NUM_SL2_BANKS firewall bank IDs for the accelerator
 * \param regionIdx  Firewall region index
 * \param startAddr  Start address (MUST be 16KB aligned)
 * \param size       Size in bytes (MUST be 16KB aligned)
 * \param ipPrivId   IP Accelerator Privilege ID
 * \param r5PrivId   R5 Core Privilege ID
 * \param focoPrivId FOCO Privilege ID (0 if not used)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 *
 * \note Firewall Control Register (0x000A)
 *       - Bit 1 (0x002): Foreground mode (use foreground permissions)
 *       - Bit 3 (0x008): Enable background region enforcement
 *       - Bit 8: NOT set (no lock bit for test flexibility)
 *       Setting to 0x000A ensures ONLY listed privilege IDs can access the region
 */
static vx_status appConfigureSl2Firewall(
    const uint16_t *fwlIds,
    uint8_t regionIdx,
    uint64_t startAddr,
    uint32_t size,
    uint8_t ipPrivId,
    uint8_t r5PrivId,
    uint8_t focoPrivId)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t sciclient_status;
    uint32_t bank;
    uint32_t numPerms;

    /* Verify 16KB alignment - SL2 bank requirement (also satisfies 4KB firewall requirement) */
    if (((startAddr & 0x3FFFU) != 0U) || ((size & 0x3FFFU) != 0U))
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] ERROR: Not 16KB-aligned! addr=0x%" PRIx64 " size=0x%x\n",
                 (uint64_t)startAddr, size);
        status = (vx_status)VX_FAILURE;
    }
    else
    {

    /* Determine number of permission registers: 2 for VPAC, 3 for DMPAC (with FOCO) */
    numPerms = (focoPrivId != 0U) ? 3U : 2U;

    /* Configure all SL2 banks with identical firewall settings */
    for (bank = 0; bank < VHWA_NUM_SL2_BANKS; bank++)
    {
        struct tisci_msg_fwl_set_firewall_region_req fwl_req = {0};
        struct tisci_msg_fwl_set_firewall_region_resp fwl_resp = {0};

        /* Setup firewall region request */
        fwl_req.fwl_id = fwlIds[bank];
        fwl_req.region = regionIdx;
        fwl_req.control = 0x000Au;  /* Enable=0xA, Unlocked, Background=0 (enforcement) */

        /* Grant permissions: IP accelerator, R5 core, and optionally FOCO
         * Pattern from working SDE config: permissions[0]=208, [1]=213, [2]=212 */
        fwl_req.permissions[0] = ((uint32_t)ipPrivId << 16U) | 0xFFFFU;
        fwl_req.permissions[1] = ((uint32_t)r5PrivId << 16U) | 0xFFFFU;
        fwl_req.permissions[2] = (focoPrivId != 0U) ? (((uint32_t)focoPrivId << 16U) | 0xFFFFU) : 0U;

        fwl_req.n_permission_regs = numPerms;
        fwl_req.start_address = startAddr;
        if(size > 0U)
        {
            fwl_req.end_address = (startAddr + (uint64_t)size) - (uint64_t)1U;
        }
        else
        {
            fwl_req.end_address = startAddr;
        }

        sciclient_status = Sciclient_firewallSetRegion(&fwl_req, &fwl_resp, FWL_TIMEOUT_VALUE);
        if (sciclient_status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "[FWL] SetRegion failed: bank=%d, region=%d, status=%d\n",
                     bank, regionIdx, sciclient_status);
            status = (vx_status)VX_FAILURE;
            break;
        }
    }
    }

    return status;
}

/* ========================================================================== */
/*              Public VPAC Firewall API                                      */
/* ========================================================================== */

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
 * \param r5PrivId   R5 Core Privilege ID
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
    uint8_t focoPrivId)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint8_t actualR5PrivId = r5PrivId;

#if defined(SOC_J784S4) || defined(SOC_J742S2)
    /* Override R5 privilege ID based on VPAC instance for J784S4/J742S2 */
    if (vpacInstId == 0U)
    {
        actualR5PrivId = TISCI_PRIV_ID_MAIN_0_R5_0;  /* VPAC0 uses MCU2_0 (R5_0) */
    }
    else if (vpacInstId == 1U)
    {
        actualR5PrivId = TISCI_PRIV_ID_MAIN_2_R5_0;  /* VPAC1 uses MCU4_0 (R5_2) */
    }
    else
    {
        VX_PRINT(VX_ZONE_WARNING, "[FWL] Invalid VPAC instance ID: %d\n", vpacInstId);
        status = (vx_status)VX_FAILURE;
    }
#endif

    if (status == (vx_status)VX_SUCCESS)
    {
        status = appConfigureSl2Firewall(
            VPAC_FWL_IDS,
            regionIdx,
            startAddr,
            size,
            ipPrivId,
            actualR5PrivId,
            focoPrivId);
    }

    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] VPAC%d Enable FAILED: region=%d\n", vpacInstId, regionIdx);
    }
    else
    {
        /* Mark firewall region as enabled for LBIST state tracking */
        if ((vpacInstId < MAX_VPAC_INSTANCES) && (regionIdx < MAX_VPAC_REGIONS))
        {
            gVpacFirewallEnabled[vpacInstId][regionIdx] = 1U;
        }
    }

    return status;
}

/* ========================================================================== */
/*              Public DMPAC Firewall API                                     */
/* ========================================================================== */

/**
 * \brief Configure SL2 firewall for DMPAC
 *
 * Configures hardware firewall to protect DMPAC SL2 memory region.
 * Called once during system initialization via remote service.
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0)
 * \param regionIdx   Firewall region index (0-2 for DOF_MAIN/DOF_REF/SDE)
 * \param startAddr   Start address (MUST be 16KB aligned)
 * \param size        Size in bytes (MUST be 16KB aligned)
 * \param ipPrivId    IP Accelerator Privilege ID
 * \param r5PrivId    R5 Core Privilege ID
 * \param focoPrivId  FOCO Privilege ID (0 if not used)
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
    uint8_t focoPrivId)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = appConfigureSl2Firewall(
        DMPAC_FWL_IDS,
        regionIdx,
        startAddr,
        size,
        ipPrivId,
        r5PrivId,
        focoPrivId);

    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] DMPAC%d Enable FAILED: region=%d\n", dmpacInstId, regionIdx);
    }
    else
    {
        /* Mark firewall region as enabled for LBIST state tracking */
        if (regionIdx < MAX_DMPAC_REGIONS)
        {
            gDmpacFirewallEnabled[regionIdx] = 1U;
        }
    }

    return status;
}

/* ========================================================================== */
/*              Firewall Disable APIs                                         */
/* ========================================================================== */

/**
 * \brief Disable SL2 Firewall for VPAC/DMPAC
 *
 * Disables hardware firewall by clearing the control register.
 * Called during cleanup or when firewall protection is no longer needed.
 *
 * \param fwlIds     Array of VHWA_NUM_SL2_BANKS firewall bank IDs for the accelerator
 * \param regionIdx  Firewall region index
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
static vx_status appDisableSl2Firewall(
    const uint16_t *fwlIds,
    uint8_t regionIdx)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t sciclient_status;
    uint32_t bank;

    /* Disable firewall for all SL2 banks */
    for (bank = 0; bank < VHWA_NUM_SL2_BANKS; bank++)
    {
        struct tisci_msg_fwl_set_firewall_region_req fwl_req = {0};
        struct tisci_msg_fwl_set_firewall_region_resp fwl_resp = {0};

        /* Setup firewall region request */
        fwl_req.fwl_id = fwlIds[bank];
        fwl_req.region = regionIdx;
        fwl_req.control = 0x00u;  /* Disable firewall */
        fwl_req.n_permission_regs = 0;
        fwl_req.start_address = 0;
        fwl_req.end_address = 0;

        sciclient_status = Sciclient_firewallSetRegion(&fwl_req, &fwl_resp, FWL_TIMEOUT_VALUE);
        if (sciclient_status != 0)
        {
            VX_PRINT(VX_ZONE_ERROR, "[FWL] Disable failed: bank=%d, region=%d, status=%d\n",
                     bank, regionIdx, sciclient_status);
            status = (vx_status)VX_FAILURE;
            break;
        }
    }

    return status;
}

/**
 * \brief Disable SL2 firewall for VPAC
 *
 * Disables hardware firewall for VPAC SL2 memory region.
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 * \param regionIdx  Firewall region index (0-3 for LDC/MSC/NF/VISS)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
vx_status appVpacFirewallDisable(
    uint8_t vpacInstId,
    uint8_t regionIdx)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = appDisableSl2Firewall(VPAC_FWL_IDS, regionIdx);

    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] VPAC%d Disable FAILED: region=%d\n", vpacInstId, regionIdx);
    }
    else
    {
        /* Mark firewall region as disabled for LBIST state tracking */
        if ((vpacInstId < MAX_VPAC_INSTANCES) && (regionIdx < MAX_VPAC_REGIONS))
        {
            gVpacFirewallEnabled[vpacInstId][regionIdx] = 0U;
        }
    }

    return status;
}

/**
 * \brief Disable SL2 firewall for DMPAC
 *
 * Disables hardware firewall for DMPAC SL2 memory region.
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0)
 * \param regionIdx   Firewall region index (0-2 for DOF_MAIN/DOF_REF/SDE)
 *
 * \return VX_SUCCESS on success, VX_FAILURE on error
 */
vx_status appDmpacFirewallDisable(
    uint8_t dmpacInstId,
    uint8_t regionIdx)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = appDisableSl2Firewall(DMPAC_FWL_IDS, regionIdx);

    if (status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] DMPAC%d Disable FAILED: region=%d\n", dmpacInstId, regionIdx);
    }
    else
    {
        /* Mark firewall region as disabled for LBIST state tracking */
        if (regionIdx < MAX_DMPAC_REGIONS)
        {
            gDmpacFirewallEnabled[regionIdx] = 0U;
        }
    }

    return status;
}

/**
 * \brief Disable all VPAC SL2 firewall regions for LBIST
 *
 * Disables all SL2 firewall regions for a specific VPAC instance before LBIST.
 * Only disables regions that are currently enabled (tracked via state).
 * This function can be called from MCU1_0 (safety application) via IPC
 * before initiating LBIST to ensure no firewall violations occur.
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 *
 * \return VX_SUCCESS if all enabled firewalls disabled successfully
 *         VX_FAILURE if any firewall disable operation fails
 */
vx_status appVpacFirewallDisableAll(uint8_t vpacInstId)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status ret;
    uint8_t regionIdx;
    uint8_t numDisabled = 0U;

    VX_PRINT(VX_ZONE_INFO, "[FWL] Disabling VPAC%d SL2 firewalls for LBIST\n", vpacInstId);

    if (vpacInstId >= MAX_VPAC_INSTANCES)
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] Invalid VPAC instance ID: %d\n", vpacInstId);
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* Disable only VPAC firewall regions that are currently enabled (LDC, MSC, NF, VISS) */
        /* IMPORTANT: Call internal disable to preserve state flags for LBIST recovery */
        for (regionIdx = VPAC_FWL_REGION_LDC; regionIdx <= VPAC_FWL_REGION_VISS; regionIdx++)
        {
            /* Only disable if firewall was previously enabled */
            if (gVpacFirewallEnabled[vpacInstId][regionIdx] != 0U)
            {
                /* Call internal disable WITHOUT clearing the state flag */
                ret = appDisableSl2Firewall(VPAC_FWL_IDS, regionIdx);
                if (ret != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "[FWL] Failed to disable VPAC%d region %d\n", vpacInstId, regionIdx);
                    status = (vx_status)VX_FAILURE;
                    /* Continue disabling others even if one fails */
                }
                else
                {
                    numDisabled++;
                    /* NOTE: We do NOT clear gVpacFirewallEnabled[vpacInstId][regionIdx] here */
                    /* Flag stays as 1 to indicate "should be enabled after LBIST" */
                }
            }
        }

        if (numDisabled == 0U)
        {
            VX_PRINT(VX_ZONE_WARNING, "[FWL] No VPAC%d firewalls were enabled, nothing to disable\n", vpacInstId);
        }
        else if (status == (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_INFO, "[FWL] Disabled %d VPAC%d SL2 firewalls for LBIST (state flags preserved)\n", numDisabled, vpacInstId);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "[FWL] Some VPAC%d firewalls failed to disable\n", vpacInstId);
        }
    }

    return status;
}

/**
 * \brief Disable all DMPAC SL2 firewall regions for LBIST
 *
 * Disables all SL2 firewall regions for DMPAC before LBIST.
 * Only disables regions that are currently enabled (tracked via state).
 * This function can be called from MCU1_0 (safety application) via IPC
 * before initiating LBIST to ensure no firewall violations occur.
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0, only 1 DMPAC exists)
 *
 * \return VX_SUCCESS if all enabled firewalls disabled successfully
 *         VX_FAILURE if any firewall disable operation fails
 */
vx_status appDmpacFirewallDisableAll(uint8_t dmpacInstId)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status ret;
    uint8_t regionIdx;
    uint8_t numDisabled = 0U;

    VX_PRINT(VX_ZONE_INFO, "[FWL] Disabling DMPAC%d SL2 firewalls for LBIST\n", dmpacInstId);

    /* Note: Only 1 DMPAC instance exists for all devices */
    if (dmpacInstId != 0U)
    {
        VX_PRINT(VX_ZONE_ERROR, "[FWL] Invalid DMPAC instance ID: %d (only DMPAC0 exists)\n", dmpacInstId);
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* Disable only DMPAC firewall regions that are currently enabled (DOF_MAIN, DOF_REFERENCE, SDE) */
        /* IMPORTANT: Call internal disable to preserve state flags for LBIST recovery */
        for (regionIdx = DMPAC_FWL_REGION_DOF_MAIN; regionIdx <= DMPAC_FWL_REGION_SDE; regionIdx++)
        {
            /* Only disable if firewall was previously enabled */
            if (gDmpacFirewallEnabled[regionIdx] != 0U)
            {
                /* Call internal disable WITHOUT clearing the state flag */
                ret = appDisableSl2Firewall(DMPAC_FWL_IDS, regionIdx);
                if (ret != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "[FWL] Failed to disable DMPAC%d region %d\n", dmpacInstId, regionIdx);
                    status = (vx_status)VX_FAILURE;
                    /* Continue disabling others even if one fails */
                }
                else
                {
                    numDisabled++;
                    /* NOTE: We do NOT clear gDmpacFirewallEnabled[regionIdx] here */
                    /* Flag stays as 1 to indicate "should be enabled after LBIST" */
                }
            }
        }

        if (numDisabled == 0U)
        {
            VX_PRINT(VX_ZONE_WARNING, "[FWL] No DMPAC%d firewalls were enabled, nothing to disable\n", dmpacInstId);
        }
        else if (status == (vx_status)VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_INFO, "[FWL] Disabled %d DMPAC%d SL2 firewalls for LBIST (state flags preserved)\n", numDisabled, dmpacInstId);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "[FWL] Some DMPAC%d firewalls failed to disable\n", dmpacInstId);
        }
    }

    return status;
}

/**
 * \brief Query if VPAC firewall region should be enabled after LBIST
 *
 * Checks the state tracking to determine if a VPAC firewall region was
 * enabled before LBIST and should be re-enabled during reconfiguration.
 *
 * \param vpacInstId VPAC instance ID (0 for VPAC0, 1 for VPAC1 on J784S4)
 * \param regionIdx  Firewall region index (0-3 for LDC/MSC/NF/VISS)
 *
 * \return 1 if firewall should be enabled (was enabled before LBIST)
 *         0 if firewall should NOT be enabled (was disabled before LBIST)
 */
uint8_t appVpacFirewallShouldEnable(uint8_t vpacInstId, uint8_t regionIdx)
{
    uint8_t result = 0U;

    if ((vpacInstId < MAX_VPAC_INSTANCES) && (regionIdx < MAX_VPAC_REGIONS))
    {
        result = gVpacFirewallEnabled[vpacInstId][regionIdx];
    }

    return result;
}

/**
 * \brief Query if DMPAC firewall region should be enabled after LBIST
 *
 * Checks the state tracking to determine if a DMPAC firewall region was
 * enabled before LBIST and should be re-enabled during reconfiguration.
 *
 * \param dmpacInstId DMPAC instance ID (0 for DMPAC0, only 1 DMPAC exists)
 * \param regionIdx   Firewall region index (0-2 for DOF_MAIN/DOF_REF/SDE)
 *
 * \return 1 if firewall should be enabled (was enabled before LBIST)
 *         0 if firewall should NOT be enabled (was disabled before LBIST)
 */
uint8_t appDmpacFirewallShouldEnable(uint8_t dmpacInstId, uint8_t regionIdx)
{
    uint8_t result = 0U;

    if ((dmpacInstId == 0U) && (regionIdx < MAX_DMPAC_REGIONS))
    {
        result = gDmpacFirewallEnabled[regionIdx];
    }

    return result;
}

#endif /* VPAC3L */
