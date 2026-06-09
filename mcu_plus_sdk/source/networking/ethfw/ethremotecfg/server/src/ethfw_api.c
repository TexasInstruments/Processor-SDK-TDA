/*
 *
 * Copyright (c) 2020-2024 Texas Instruments Incorporated
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

/*!
 * \file ethfw_api.c
 *
 * \brief Ethernet Firmware remote config server API.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x101

/* Enet Driver header files */
#include <enet.h>
#include <include/per/cpsw.h>
#include <utils/include/enet_mcm.h>
#include <utils/include/enet_apputils.h>

/* EthFw header files */
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>
#include <utils/ethfw_common/include/ethfw_utils.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <ethremotecfg/server/include/ethfw.h>
#include <ethremotecfg/server/include/ethfw_virtport.h>
#include "cpsw_proxy_server.h"
#include "ethfw_mcast_priv.h"
#include "ethfw_vlan_priv.h"
#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
#include "ethfw_arp_priv.h"
#endif
#if defined(ETHFW_VEPA_SUPPORT)
#include "ethfw_vepa_priv.h"
#endif

#if defined(ETHFW_GPTP_SUPPORT)
#include <ethremotecfg/server/include/ethfw_tsn.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! Month substring offset in date string */
#define ETHFWVERSION_OFFSET_MONTH                    (0)

/*! Date substring offset in date string */
#define ETHFWVERSION_OFFSET_DATE                     (4)

/*! Year substring offset in data string */
#define ETHFWVERSION_OFFSET_YEAR                     (7)

/*! Month substring offset in date string */
#define ETHFWVERSION_OFFSET_HOUR                     (0)

/*! Date substring offset in date string */
#define ETHFWVERSION_OFFSET_MIN                      (3)

/*! Year substring offset in data string */
#define ETHFWVERSION_OFFSET_SEC                      (6)

/*! AUTOSAR Eth driver endpoint number */
#define AUTOSAR_ETHDRIVER_DEVICE_ENDPT                (28U)

/*! AUTOSAR Eth driver endpoint-2 number */
#define AUTOSAR_ETHDRIVER_DEVICE_ENDPT2               (38U)

/*! Max VLAN id as per standard */
#define ETHFW_VLAN_ID_MAX                             (4094U)

/*! VLAN id used for host port */
#define ETHFW_HOST_PORT_VLAN_ID                       (1U)

/*! VLAN id used for all MAC ports in MAC-only mode */
#define ETHFW_MAC_ONLY_PORTS_VLAN_ID                  (0U)

/*! VLAN id used for all MAC ports in switch mode (non MAC-only mode) */
#define ETHFW_SWITCH_PORTS_VLAN_ID                    (3U)

/*! Max number of CPSW MAC ports supported */
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
#define ETHFW_MAC_PORT_MAX                            (8U)
#else
#define ETHFW_MAC_PORT_MAX                            (4U)
#endif

/* Compile time check for error value consistency with Enet LLD (and CSL) */
#define ETHFW_UTILS_COMPILETIME_ENET_CHECK(x)         ETHFW_UTILS_COMPILETIME_ASSERT(ETHFW_##x == ENET_##x)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct EthFw_Port_s
{
    /* MAC port number */
    Enet_MacPort macPort;

    /* Port VLAN config */
    EnetPort_VlanCfg vlanCfg;
} EthFw_Port;

typedef struct EthFw_Obj_s
{
    /* Core Id */
    uint32_t coreId;

    /* Enet instance type */
    Enet_Type enetType;

    /* Enet instance id */
    uint32_t instId;

    /* Enet handle */
    Enet_Handle hEnet;

    /* CPSW configuration */
    Cpsw_Cfg cpswCfg;

    /* Firmware version */
    EthFw_Version version;

    /* Port mask of all enabled MAC ports */
    uint32_t enabledPortMask;

    /* Port mask of all MAC-only ports */
    uint32_t macOnlyPortMask;

    /* Port mask of all non MAC-only ports */
    uint32_t switchPortMask;

    /* MAC ports owned by EthFw */
    EthFw_Port ports[ENET_MAC_PORT_NUM];

    /* Number of MAC ports owned by EthFw */
    uint32_t numPorts;

    /* Virtual port configuration */
    EthFwVirtPort_VirtPortCfg virtPortCfg[ETHFW_REMOTE_CLIENT_MAX];

    /* Number of valid virtual port configuration entries */
    uint32_t numVirtPorts;

    /* Default VLAN id to be used for MAC ports configured in MAC-only mode */
    uint16_t dfltVlanIdMacOnlyPorts;

    /* Default VLAN id to be used for MAC ports configured in switch mode (non MAC-only) */
    uint16_t dfltVlanIdSwitchPorts;

    /* Multiclient Manager (MCM) handle */
    EnetMcm_CmdIf mcmCmdIf;

    /* Callback function for application to set port link parameters */
    EthFw_setPortCfg setPortCfg;

    /* EthFw status */
    EthRemoteCfg_ServerStatus serverStatus;

    /* Pass error packets to host port */
    bool passErrPkt;
} EthFw_Obj;

typedef struct EthFw_Autosar_EpId_s
{
    /*! Remote core id */
    uint32_t remoteCoreId;

    /*! Remote Endpoint id */
    uint32_t remoteEndptId;
} EthFw_Autosar_EpId;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static bool EthFw_isMacOnlyPort(Enet_MacPort macPort);

static int32_t EthFw_initMcm(void);

static void EthFw_deinitMcm(void);

static void EthFw_initLinkArgs(EnetPer_PortLinkCfg *linkArgs,
                               Enet_MacPort macPort);

static int32_t EthFw_setAleBcastEntry(void);

static void EthFw_getMcmCmdIfCb(Enet_Type enetType,
                                EnetMcm_CmdIf **pMcmCmdIfHandle);

static void EthFw_getDeviceData(EthRemoteCfg_DeviceData *ethdevData);

static void EthFw_handleProfileInfoNotify(uint32_t host_id,
                                          Enet_Handle hEnet,
                                          Enet_Type enetType,
                                          uint32_t core_key,
                                          EthRemoteCfg_NotifyType notifyid,
                                          uint8_t *notify_info,
                                          uint32_t notify_info_len);

EthRemoteCfg_ServerStatus EthFw_getStatus(void);
void EthFw_setStatus(EthRemoteCfg_ServerStatus serverStatus);

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*! Ethernet Firmware object */
EthFw_Obj gEthFwObj;

/* EthFw RM: TX channel, RX flow and MAC address partitioning */
static EnetRm_ResPrms gEthFw_rmResPrms =
{
    .coreResInfo =
    {
        [0] =
        {
            .coreId        = IPC_MPU1_0,
            .numTxCh       = 0U,
            .numRxFlows    = 0U,
            .numMacAddress = 0U,
            .numHwPush     = 2U,
        },
        [1] =
        {
            .coreId        = IPC_MCU1_0,
            .numTxCh       = 0U,
            .numRxFlows    = 0U,
            .numMacAddress = 0U,
            .numHwPush     = 2U,
        },
        [2] =
        {
            .coreId        = IPC_MCU2_0,
            .numTxCh       = 0U,
            .numRxFlows    = 0U,
            .numMacAddress = 0U,
            .numHwPush     = 2U,
        },
        [3] =
        {
            .coreId        = IPC_MCU2_1,
            .numTxCh       = 0U,
            .numRxFlows    = 0U,
            .numMacAddress = 0U,
            .numHwPush     = 1U,
        },
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
        [4] =
        {
            .coreId        = IPC_MCU3_0,
            .numTxCh       = 0U,
            .numRxFlows    = 0U,
            .numMacAddress = 0U,
            .numHwPush     = 1U,
        },
        [5] =
        {
            .coreId        = IPC_MCU3_1,
            .numTxCh       = 0U,
            .numRxFlows    = 0U,
            .numMacAddress = 0U,
        },
#endif
    },
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
    .numCores = 6U,
#else
    .numCores = 4U,
#endif
    .isStaticTxChanAllocated = BFALSE,
};

/* EthFw IOCTLs: allow all on all cores */
static const EnetRm_IoctlPermissionTable gEthFw_rmIoctlPerm =
{
    .defaultPermittedCoreMask = (ENET_BIT(IPC_MPU1_0) |
                                 ENET_BIT(IPC_MCU2_0) |
                                 ENET_BIT(IPC_MCU2_1) |
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
                                 ENET_BIT(IPC_MCU3_0) |
                                 ENET_BIT(IPC_MCU3_1) |
#endif
                                 ENET_BIT(IPC_MCU1_0)),
    .numEntries = 0,
};

/* IPC endpoints used for AUTOSAR virtual clients */
static EthFw_Autosar_EpId gEthFw_autosarEndptId[] =
{
    {
        .remoteCoreId  = IPC_MCU2_1,
        .remoteEndptId = AUTOSAR_ETHDRIVER_DEVICE_ENDPT,
    },
    {
        .remoteCoreId  = IPC_MCU1_0,
        .remoteEndptId = AUTOSAR_ETHDRIVER_DEVICE_ENDPT2,
    },
};

/* Policer table partition into chunks of different size each having different
 * priority CPSW_ALE_POLICER_PARTITION_DEFAULT having lowest and
 * CPSW_ALE_POLICER_PARTITION_LEVEL_1 having the highest priority */
static uint32_t gEthFw_policerTablePartSize[CPSW_ALE_POLICER_TABLE_PART_MAX] =
{
    /* This partition is used for private VLAN by default */
    [CPSW_ALE_POLICER_PARTITION_LEVEL_1] = 10U,
    /* This partition is used for custom policers by default.
     * These are policers which are created by ethfw application for each virtual port
     * on specific rx flows (extended).
     * Custom policer configuration can be changed from ethfw application */
    [CPSW_ALE_POLICER_PARTITION_LEVEL_2] = ETHFW_UTILS_NUM_CUSTOM_POLICERS,
    [CPSW_ALE_POLICER_PARTITION_LEVEL_3] = 10U,
    [CPSW_ALE_POLICER_PARTITION_LEVEL_4] = 0U,
    /* Give the unused/default partition size to be 0
     * Partitions with size 0 are clubbed to default partition */
    [CPSW_ALE_POLICER_PARTITION_DEFAULT] = 0U
};
/* Note: Sum of partition sizes must be <= Total number of policer entries available */

EthFwPortMirroring_Cfg gPortMirCfg = {.mirroringType = DISABLE_PORT_MIRRORING};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void EthFw_compileTimeChecks(void)
{
    /* Verify that ETHFW error types are consistent with Enet LLD error types */
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(SOK);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(SINPROGRESS);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EFAIL);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EBADARGS);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EINVALIDPARAMS);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(ETIMEOUT);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EALLOC);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EUNEXPECTED);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EBUSY);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EALREADYOPEN);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(EPERM);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(ENOTSUPPORTED);
    ETHFW_UTILS_COMPILETIME_ENET_CHECK(ENOTFOUND);
}

static void EthFw_initAleCfg(CpswAle_Cfg *aleCfg)
{
    /* ALE configuration */
    aleCfg->modeFlags = CPSW_ALE_CFG_MODULE_EN;
#if defined(ETHFW_VEPA_SUPPORT)
    aleCfg->modeFlags |= CPSW_ALE_CFG_MULTIHOST;
#endif

    aleCfg->agingCfg.autoAgingEn     = BTRUE;
    aleCfg->agingCfg.agingPeriodInMs = 1000;

    aleCfg->nwSecCfg.vid0ModeEn = BFALSE;

    aleCfg->vlanCfg.aleVlanAwareMode           = BTRUE;
    aleCfg->vlanCfg.cpswVlanAwareMode          = BTRUE;
    aleCfg->vlanCfg.unknownUnregMcastFloodMask = 0U;
    aleCfg->vlanCfg.unknownRegMcastFloodMask   = 0U;
    aleCfg->vlanCfg.unknownVlanMemberListMask  = CPSW_ALE_ALL_MACPORTS_MASK;
    aleCfg->vlanCfg.autoLearnWithVlan          = BFALSE;

    /* ALE policer configuration */
    aleCfg->policerGlobalCfg.policingEn         = BTRUE;
    aleCfg->policerGlobalCfg.yellowDropEn       = BFALSE;
    aleCfg->policerGlobalCfg.redDropEn          = BTRUE;
    aleCfg->policerGlobalCfg.policerNoMatchMode = CPSW_ALE_POLICER_NOMATCH_MODE_GREEN;

    /* ALE policer partition configuration */
    memcpy(aleCfg->policerTablePartSize, gEthFw_policerTablePartSize, sizeof(gEthFw_policerTablePartSize));
}

static int32_t EthFw_setDscpPriorityMapRegisters(void)
{
    EnetPort_DscpPriorityMap setHostInArgs;
    Enet_IoctlPrms prms;
    uint32_t i;
    int32_t status;

    setHostInArgs.dscpIPv4En = BTRUE;
    setHostInArgs.dscpIPv6En = BTRUE;

    for (i = 0U; i < ENET_ARRAYSIZE(setHostInArgs.tosMap); i++)
    {
        setHostInArgs.tosMap[i] = (i % 8U);
    }

    ENET_IOCTL_SET_IN_ARGS(&prms, &setHostInArgs);

    ENET_IOCTL(gEthFwObj.hEnet,
               gEthFwObj.coreId,
               ENET_HOSTPORT_IOCTL_SET_INGRESS_DSCP_PRI_MAP,
               &prms,
               status);

    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to set ingress DSCP priority mapping");
    return status;
}

static int32_t EthFw_getDfltVlanId(const EthFw_Config *config)
{
    int32_t status = ENET_SOK;

    if (config->dfltVlanIdMacOnlyPorts > ETHFW_VLAN_ID_MAX)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Default VLAN id %u for MAC-only ports is out-of-range",
                     config->dfltVlanIdMacOnlyPorts);
    }

    if (config->dfltVlanIdSwitchPorts > ETHFW_VLAN_ID_MAX)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Default VLAN id %u for switch ports is out-of-range",
                     config->dfltVlanIdSwitchPorts);
    }

    if (config->dfltVlanIdMacOnlyPorts == config->dfltVlanIdSwitchPorts)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Default VLAN Id should not be same for MAC-only and switch ports (%u)",
                     config->dfltVlanIdSwitchPorts);
    }

    ETHFWTRACE_WARN_IF((config->dfltVlanIdMacOnlyPorts != 0U),
                       "Default VLAN of MAC-only ports is %u, "
                       "promiscuous mode will not be functional if VLAN is not 0",
                       config->dfltVlanIdMacOnlyPorts);

    if (status == ENET_SOK)
    {
        gEthFwObj.dfltVlanIdMacOnlyPorts = config->dfltVlanIdMacOnlyPorts;
        gEthFwObj.dfltVlanIdSwitchPorts  = config->dfltVlanIdSwitchPorts;
    }

    return status;
}

static int32_t EthFw_getPortConfig(const EthFw_Config *config)
{
    EthRemoteCfg_VirtPort virtPort;
    Enet_MacPort macPort;
    uint32_t i;
    int32_t status = ENET_SOK;
    uint32_t j;

    if (config->setPortCfg == NULL)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Invalid setPortCfg callback");
    }

    if (config->numPorts > ETHFW_MAC_PORT_MAX)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Too many MAC ports requested (%u), max is %u",
                     config->numPorts, ETHFW_MAC_PORT_MAX);
    }

    if (config->numVirtPorts > ETHFW_REMOTE_CLIENT_MAX)
    {
        status = ENET_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "Too many virtual ports requested (%u), max is %u",
                       config->numVirtPorts, ETHFW_REMOTE_CLIENT_MAX);
    }

    for (i = 0U; i <= ENET_MAC_PORT_NUM; i++)
    {
        /* Default VLAN of all ports is set to ETHFW_VLAN_ID_MAX, as 0U is reserved for MAC only ports */
        gEthFwObj.cpswCfg.aleCfg.portCfg[i].pvidCfg.vlanIdInfo.vlanId = ETHFW_VLAN_ID_MAX;
    }

    /* Initializing absolute tx channels for each core */
    for (i = 0U; i < gEthFw_rmResPrms.numCores; i++)
    {
        for (j = 0U; j < ENET_CFG_TX_CHANNELS_NUM; j++)
        {
            gEthFw_rmResPrms.coreResInfo[i].txCh[j] = ENET_RM_TX_CH_ANY;
        }
    }

    if (status == ENET_SOK)
    {
        gEthFwObj.setPortCfg = config->setPortCfg;

        /* Get the port mask of all enabled MAC ports */
        gEthFwObj.numPorts = config->numPorts;
        for (i = 0U; i < gEthFwObj.numPorts; i++)
        {
            gEthFwObj.ports[i].macPort = config->ports[i];
            macPort = gEthFwObj.ports[i].macPort;

            gEthFwObj.enabledPortMask |= ENET_MACPORT_MASK(macPort);
        }

        /* Get the port mask of all ports in MAC-only mode */
        gEthFwObj.numVirtPorts = config->numVirtPorts;
        gEthFw_rmResPrms.isStaticTxChanAllocated = config->isStaticTxChanAllocated;
        for (i = 0U; i < gEthFwObj.numVirtPorts; i++)
        {
            gEthFwObj.virtPortCfg[i] = config->virtPortCfg[i];
            virtPort = gEthFwObj.virtPortCfg[i].portId;

            if (EthFwVirtPort_isMacPort(virtPort))
            {
                macPort = EthFwVirtPort_getMacPort(virtPort);

                gEthFwObj.macOnlyPortMask |= ENET_MACPORT_MASK(macPort);
            }
        }

        gEthFwObj.switchPortMask = (gEthFwObj.enabledPortMask &
                                    ~gEthFwObj.macOnlyPortMask);
    }

    return status;
}

static EthFw_Port *EthFw_getMacPortConfig(Enet_MacPort macPort)
{
    EthFw_Port *port = NULL;
    uint32_t i;

    for (i = 0U; i < gEthFwObj.numPorts; i++)
    {
        if (gEthFwObj.ports[i].macPort == macPort)
        {
            port = &gEthFwObj.ports[i];
            break;
        }
    }

    return port;
}

static void EthFw_setPortVlan(void)
{
    EthFw_Port *ethFwPort;
    Enet_MacPort macPort;
    EnetPort_VlanCfg *vlanCfg;
    uint32_t i;

    for (i = 0U; i < gEthFwObj.numPorts; i++)
    {
        macPort = gEthFwObj.ports[i].macPort;

        ethFwPort = EthFw_getMacPortConfig(macPort);
        if (ethFwPort != NULL)
        {
            vlanCfg = &ethFwPort->vlanCfg;

            if (EthFw_isMacOnlyPort(macPort))
            {
                vlanCfg->portVID = gEthFwObj.dfltVlanIdMacOnlyPorts;
            }
            else
            {
                vlanCfg->portVID = gEthFwObj.dfltVlanIdSwitchPorts;
            }
            vlanCfg->portPri = 0U;
            vlanCfg->portCfi = 0U;
        }
    }
}

static int32_t EthFw_setupVlan(const EthFw_Config *config)
{
    EthFwVlan_Cfg vlanCfg;
    int32_t status = ENET_SOK;

    vlanCfg.vlanCfg                = config->vlanCfg;
    vlanCfg.numStaticVlans         = config->numStaticVlans;
    vlanCfg.dfltVlanIdSwitchPorts  = gEthFwObj.dfltVlanIdSwitchPorts;
    vlanCfg.dfltVlanIdMacOnlyPorts = gEthFwObj.dfltVlanIdMacOnlyPorts;
    vlanCfg.switchPortMask         = gEthFwObj.switchPortMask;
    vlanCfg.macOnlyPortMask        = gEthFwObj.macOnlyPortMask;
    vlanCfg.defaultPortMask        = config->defaultPortMask;
    vlanCfg.defaultVirtPortMask    = config->defaultVirtPortMask;
    vlanCfg.dVlanSwtFwdEn          = config->dVlanSwtFwdEn;

    if (gEthFwObj.hEnet != NULL)
    {
        status = EthFwVlan_init(gEthFwObj.hEnet, &vlanCfg);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Incorrect VLAN configuration");
    }
    else
    {
        status = ENET_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to get Enet handle");
    }

    return status;
}

static bool EthFw_isMacOnlyPort(Enet_MacPort macPort)
{
    bool isMacOnly = BFALSE;

    if ((gEthFwObj.macOnlyPortMask & ENET_MACPORT_MASK(macPort)) != 0U)
    {
        isMacOnly = BTRUE;
    }

    return isMacOnly;
}

static void EthFw_setPortMode(void)
{
    Enet_MacPort macPort;
    CpswAle_Cfg *aleCfg = &gEthFwObj.cpswCfg.aleCfg;
    CpswAle_PortVlanCfg *pvidCfg;
    CpswAle_PortMacModeCfg *macModeCfg;
    CpswAle_PortLearningSecurityCfg *learningCfg;
    CpswAle_PortVlanSecurityCfg *vlanSecCfg;
    uint32_t aleSwitchPortMask = 0U;
    uint32_t aleMacOnlyPortMask = 0U;
    uint32_t alePort;
    uint32_t i;

    aleSwitchPortMask  = (gEthFwObj.switchPortMask << 1U);
    aleMacOnlyPortMask = (gEthFwObj.macOnlyPortMask << 1U);

    /* Reset MAC-only and learning config for all enabled ports. It will be
     * overwritten as needed right after */
    for (i = 0U; i < gEthFwObj.numPorts; i++)
    {
        macPort = gEthFwObj.ports[i].macPort;
        alePort = CPSW_ALE_MACPORT_TO_ALEPORT(macPort);

        macModeCfg  = &aleCfg->portCfg[alePort].macModeCfg;
        learningCfg = &aleCfg->portCfg[alePort].learningCfg;
        pvidCfg     = &aleCfg->portCfg[alePort].pvidCfg;
        vlanSecCfg  = &aleCfg->portCfg[alePort].vlanCfg;

        if (EthFw_isMacOnlyPort(macPort))
        {
            macModeCfg->macOnlyCafEn = BFALSE;
            macModeCfg->macOnlyEn    = BTRUE;
            learningCfg->noLearn     = BTRUE;
            vlanSecCfg->dropUntagged = BFALSE;

            pvidCfg->vlanIdInfo.tagType  = ENET_VLAN_TAG_TYPE_INNER;
            pvidCfg->vlanIdInfo.vlanId   = gEthFwObj.dfltVlanIdMacOnlyPorts;
            pvidCfg->vlanMemberList      = aleMacOnlyPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->regMcastFloodMask   = aleMacOnlyPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->unregMcastFloodMask = aleMacOnlyPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->forceUntaggedEgressMask = aleMacOnlyPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->noLearnMask     = 0U;
            pvidCfg->vidIngressCheck = BFALSE;
            pvidCfg->limitIPNxtHdr   = BFALSE;
            pvidCfg->disallowIPFrag  = BFALSE;
        }
        else
        {
            macModeCfg->macOnlyCafEn = BFALSE;
            macModeCfg->macOnlyEn    = BFALSE;
            learningCfg->noLearn     = BFALSE;
            vlanSecCfg->dropUntagged = BFALSE;

            pvidCfg->vlanIdInfo.tagType  = ENET_VLAN_TAG_TYPE_INNER;
            pvidCfg->vlanIdInfo.vlanId   = gEthFwObj.dfltVlanIdSwitchPorts;
            pvidCfg->vlanMemberList      = aleSwitchPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->regMcastFloodMask   = aleSwitchPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->unregMcastFloodMask = 0U;
            pvidCfg->forceUntaggedEgressMask = aleSwitchPortMask | CPSW_ALE_HOST_PORT_MASK;
            pvidCfg->noLearnMask     = 0U;
            pvidCfg->vidIngressCheck = BFALSE;
            pvidCfg->limitIPNxtHdr   = BFALSE;
            pvidCfg->disallowIPFrag  = BFALSE;
        }
    }

    /* ALE host port configuration */
    pvidCfg = &aleCfg->portCfg[0].pvidCfg;
    pvidCfg->vlanIdInfo.tagType  = ENET_VLAN_TAG_TYPE_INNER;
    pvidCfg->vlanIdInfo.vlanId   = ETHFW_HOST_PORT_VLAN_ID;
    pvidCfg->vlanMemberList      = CPSW_ALE_ALL_PORTS_MASK;
    pvidCfg->regMcastFloodMask   = CPSW_ALE_ALL_PORTS_MASK;
    pvidCfg->unregMcastFloodMask = 0U;
    pvidCfg->forceUntaggedEgressMask = CPSW_ALE_ALL_PORTS_MASK;
    pvidCfg->noLearnMask     = 0U;
    pvidCfg->vidIngressCheck = BFALSE;
    pvidCfg->limitIPNxtHdr   = BFALSE;
    pvidCfg->disallowIPFrag  = BFALSE;

    learningCfg = &aleCfg->portCfg[0].learningCfg;
    learningCfg->noLearn      = BFALSE;
#if defined(ETHFW_VEPA_SUPPORT)
    learningCfg->noSaUpdateEn = BTRUE;
#endif

    vlanSecCfg = &aleCfg->portCfg[0].vlanCfg;
    vlanSecCfg->dropUntagged   = BFALSE;
#if defined(ETHFW_VEPA_SUPPORT)
    vlanSecCfg->dropDoubleVlan = BFALSE;
    vlanSecCfg->dropDualVlan   = BTRUE;
#endif
}

static void EthFw_setTxChRmInfo(uint32_t coreIdx,
                                uint32_t virtPortId)
{
    uint32_t i;
    uint32_t j = 0U;

    /* Populate the tx channel for individual core from virtual port configuration */
    for (i = 0U; i < ENET_CFG_TX_CHANNELS_NUM; i++)
    {
        if (gEthFw_rmResPrms.coreResInfo[coreIdx].txCh[i] == ENET_RM_TX_CH_ANY)
        {
            gEthFw_rmResPrms.coreResInfo[coreIdx].txCh[i] = gEthFwObj.virtPortCfg[virtPortId].txCh[j];
            j++;
        }

        /* All the tx channels for this virtual port has been added to coreResInfo */
        if (j == gEthFwObj.virtPortCfg[virtPortId].numTxCh)
        {
            break;
        }
    }
}

static uint32_t EthFw_getCoreDmaResIndex(uint32_t coreId)
{
    uint32_t coreIdx;

    for (coreIdx = 0U; coreIdx < gEthFw_rmResPrms.numCores; coreIdx++)
    {
        /* Find the first entry with the same coreId */
        if (gEthFw_rmResPrms.coreResInfo[coreIdx].coreId == coreId)
        {
            break;
        }
    }

    ETHFWTRACE_ERR_IF((coreIdx == gEthFw_rmResPrms.numCores), ETHFW_EFAIL, "Failed to find core dma resource index");
    return coreIdx;
}

static void EthFw_updateEnetRm(void)
{
    EnetRm_ResCfg *resCfg = &gEthFwObj.cpswCfg.resCfg;
    EnetRm_ResPrms *rmPrms = &resCfg->resPartInfo;
    uint32_t req = 0U;
    uint32_t coreIdx;
    uint32_t i;

    /* Add RM needed by virtual ports, each one needs:
     * - numTxCh x TX channel
     * - numRxFlow x RX flow
     * - numMacAddress x MAC address from ETHFW pool
     */
    for (i = 0U; i < gEthFwObj.numVirtPorts; i++)
    {
        coreIdx = EthFw_getCoreDmaResIndex(gEthFwObj.virtPortCfg[i].remoteCoreId);
        gEthFw_rmResPrms.coreResInfo[coreIdx].numTxCh       += gEthFwObj.virtPortCfg[i].numTxCh;
        gEthFw_rmResPrms.coreResInfo[coreIdx].numRxFlows    += gEthFwObj.virtPortCfg[i].numRxFlow;
        gEthFw_rmResPrms.coreResInfo[coreIdx].numMacAddress += gEthFwObj.virtPortCfg[i].numMacAddress;
        EthFw_setTxChRmInfo(coreIdx, i);
    }

    /* Overwriting RM with our own */
    resCfg->resPartInfo = gEthFw_rmResPrms;

    /* Compute the MAC address pool size for the virtual port allocation */
    for (i = 0U; i < rmPrms->numCores; i++)
    {
        req += rmPrms->coreResInfo[i].numMacAddress;
    }

    /* Limit pool size to the size of MAC address array */
    if (resCfg->macList.numMacAddress > ENET_ARRAYSIZE(resCfg->macList.macAddress))
    {
        resCfg->macList.numMacAddress = ENET_ARRAYSIZE(resCfg->macList.macAddress);
    }

    /* Pool size provided by application is too small, warn user about it */
    if (resCfg->macList.numMacAddress == 0U)
    {
        ETHFWTRACE_ERR(ENET_EALLOC, "Empty MAC address pool");
        EnetAppUtils_assert(BFALSE);
    }
    else if (resCfg->macList.numMacAddress < req)
    {
        ETHFWTRACE_WARN("MAC address pool size is too small (req=%u alloc=%u),"
                        "may not be sufficient depending on concurrent usage",
                        req, resCfg->macList.numMacAddress);
    }
}

void EthFw_initConfigParams(Enet_Type enetType,
                            uint32_t instId,
                            EthFw_Config *config)
{
    Cpsw_Cfg *cpswCfg = &config->cpswCfg;
    CpswAle_Cfg *aleCfg = &cpswCfg->aleCfg;
    Cpsw_VlanCfg *vlanCfg = &cpswCfg->vlanCfg;
    CpswHostPort_Cfg *hostPortCfg = &cpswCfg->hostPortCfg;
    CpswCpts_Cfg *cptsCfg = &cpswCfg->cptsCfg;
    EnetRm_ResCfg *resCfg = &cpswCfg->resCfg;

    memset(config, 0, sizeof(*config));

    /* Port mirroring configuration */
    config->portMirCfg = &gPortMirCfg;

    /* MAC port ownership */
    config->ports = NULL;
    config->numPorts = 0U;

    /* Virtual ports (remote_device based) */
    config->virtPortCfg = NULL;
    config->numVirtPorts = 0U;

    /* VLAN configuration */
    config->vlanCfg = NULL;
    config->numStaticVlans = 0U;
    config->dVlanSwtFwdEn  = BFALSE;

    /* Multicast configuration */
    config->mcastCfg.sharedMcastCfg.filterAddMacSharedCb = NULL;
    config->mcastCfg.sharedMcastCfg.filterDelMacSharedCb = NULL;
    config->mcastCfg.sharedMcastCfg.mcastCfg = NULL;
    config->mcastCfg.sharedMcastCfg.numMcast = 0U;
    config->mcastCfg.rsvdMcastCfg.mcastCfg = NULL;
    config->mcastCfg.rsvdMcastCfg.numMcast = 0U;

    /* Default VLAN ids */
    config->dfltVlanIdMacOnlyPorts = ETHFW_MAC_ONLY_PORTS_VLAN_ID;
    config->dfltVlanIdSwitchPorts  = ETHFW_SWITCH_PORTS_VLAN_ID;

#if defined(ETHFW_VEPA_SUPPORT)
    /* Initialize VEPA config params */
    EthFwVepa_initCfg(&config->vepaCfg);
#endif

#if defined(ETHFW_MONITOR_SUPPORT)
    EthFwMon_initCfg(&config->monitorCfg);
#endif

#if defined(ETHFW_GPTP_SUPPORT)
    memset(&config->ppsConfig, 0, sizeof(config->ppsConfig));
#endif

    config->passErrPkt = BFALSE;

    /* Start with CPSW LLD's default configuration */
    Enet_initCfg(enetType, instId, cpswCfg, sizeof (*cpswCfg));
    cpswCfg->dmaCfg = NULL;
    resCfg->ioctlPermissionInfo = gEthFw_rmIoctlPerm;
    resCfg->selfCoreId = EnetSoc_getCoreId();
    resCfg->macList.numMacAddress = 0U;

    /* CPTS host receive timestamp is only enabled for Demo purpose
     * And strictly advised not to use for production */
#if defined(ETHFW_EST_DEMO_SUPPORT)
    cptsCfg->hostRxTsEn = BTRUE;
#else
    /* Disable CPTS host receive timestamping as it can cause
     * MAC port lockup in packets with corrupted SFD */
    cptsCfg->hostRxTsEn = BFALSE;
#endif

    /* VLAN configuration */
    vlanCfg->vlanAware = BTRUE;

    /* Host port configuration */
    hostPortCfg->removeCrc         = BTRUE;
    hostPortCfg->padShortPacket    = BTRUE;
    hostPortCfg->passCrcErrors     = BTRUE;
    hostPortCfg->rxMtu             = 1522U;
    hostPortCfg->vlanCfg.portVID   = ETHFW_HOST_PORT_VLAN_ID;
    /* To honor the vlan/dscp packet priority instead of CPPI thread id for mapping */
    hostPortCfg->rxVlanRemapEn     = BTRUE;
    hostPortCfg->rxDscpIPv4RemapEn = BTRUE;
    hostPortCfg->rxDscpIPv6RemapEn = BTRUE;
#if defined(ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA)
    hostPortCfg->csumOffloadEn   = BFALSE;
#else
#if defined (MCU_PLUS_SDK)
    hostPortCfg->txCsumOffloadEn   = BTRUE;
#else
    hostPortCfg->csumOffloadEn     = BTRUE;
#endif
#endif

    EthFw_initAleCfg(aleCfg);
}

EthFw_Handle EthFw_init(Enet_Type enetType,
                        uint32_t instId,
                        const EthFw_Config *config)
{
    EnetDma_Cfg *udmaCfg;
    char *date = __DATE__;
    char *time = __TIME__;
    int32_t status = ENET_SOK;
#if defined(ETHFW_GPTP_SUPPORT)
    EthFwTsn_Config tsnCfg;
#endif

    EthFw_compileTimeChecks();

    EnetAppUtils_assert(config != NULL);
    EnetAppUtils_assert(config->ports != NULL);
    EnetAppUtils_assert(config->numPorts <= ENET_MAC_PORT_NUM);

    udmaCfg = (EnetDma_Cfg *)config->cpswCfg.dmaCfg;
    EnetAppUtils_assert(udmaCfg != NULL);
    EnetAppUtils_assert(udmaCfg->hUdmaDrv != NULL);

    memset(&gEthFwObj, 0, sizeof(gEthFwObj));

    /* Save config parameters */
    gEthFwObj.cpswCfg = config->cpswCfg;

    /* EthFw status */
    gEthFwObj.serverStatus = ETHREMOTECFG_SERVERSTATUS_UNINIT;

    /* Get default VLAN ids for MAC-only and switch ports */
    status = EthFw_getDfltVlanId(config);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to get default VLAN ids");

    /* Save hardware and virtual port configuration */
    if (status == ENET_SOK)
    {
        status = EthFw_getPortConfig(config);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Incorrect port configuration");
    }

    /* Set MAC port's default VLAN id */
    if (status == ENET_SOK)
    {
        EthFw_setPortVlan();
    }

    if (status == ENET_SOK)
    {
        /* Set EthFw port mode: switch or MAC-only */
        EthFw_setPortMode();

        /* Update Enet RM according to the virtual port configuration */
        EthFw_updateEnetRm();
    }

    if (status == ENET_SOK)
    {
        gEthFwObj.coreId = EnetSoc_getCoreId();
        gEthFwObj.enetType = enetType;
        gEthFwObj.instId = instId;

        /* Populate EthFw version */
        gEthFwObj.version.major = ETHREMOTECFG_FW_ETHSWITCH_VERSION_MAJOR;
        gEthFwObj.version.minor = ETHREMOTECFG_FW_ETHSWITCH_VERSION_MINOR;
        gEthFwObj.version.rev = ETHREMOTECFG_FW_ETHSWITCH_VERSION_REVISION;

        /* __DATE__ is a string constant that contains eleven characters and
         * looks like "Feb 12 1996". If the day of the month is less than
         * 10, it is padded with a space on the left */
        memcpy(&gEthFwObj.version.month[0U],
               &date[ETHFWVERSION_OFFSET_MONTH],
               ETHFW_VERSION_MONTHLEN);
        memcpy(&gEthFwObj.version.date[0U],
               &date[ETHFWVERSION_OFFSET_DATE],
               ETHFW_VERSION_DATELEN);
        memcpy(&gEthFwObj.version.year[0U],
               &date[ETHFWVERSION_OFFSET_YEAR],
               ETHFW_VERSION_YEARLEN);


        /* __TIME__ is a string in 24 hour time format */
        memcpy(&gEthFwObj.version.hour[0U],
               &time[ETHFWVERSION_OFFSET_HOUR],
               ETHFW_VERSION_HOURLEN);
        memcpy(&gEthFwObj.version.min[0U],
               &time[ETHFWVERSION_OFFSET_MIN],
               ETHFW_VERSION_MINLEN);
        memcpy(&gEthFwObj.version.sec[0U],
               &time[ETHFWVERSION_OFFSET_SEC],
               ETHFW_VERSION_SECLEN);
#if !defined(MCU_PLUS_SDK)
        /* ETHRPC_ETHSWITCH_VERSION_LAST_COMMIT is defined by the build system */
        memcpy(&gEthFwObj.version.commitHash[0U],
               ETHREMOTECFG_ETHSWITCH_VERSION_LAST_COMMIT,
               ETHFW_VERSION_COMMITSHALEN);
#endif
        gEthFwObj.version.month[ETHFW_VERSION_MONTHLEN] = '\0';
        gEthFwObj.version.date[ETHFW_VERSION_DATELEN] = '\0';
        gEthFwObj.version.year[ETHFW_VERSION_YEARLEN] = '\0';
        gEthFwObj.version.hour[ETHFW_VERSION_HOURLEN] = '\0';
        gEthFwObj.version.min[ETHFW_VERSION_MINLEN] = '\0';
        gEthFwObj.version.sec[ETHFW_VERSION_SECLEN] = '\0';
        gEthFwObj.version.commitHash[ETHFW_VERSION_COMMITSHALEN] = '\0';
    }

    /* Initialize MCM */
    if (status == ENET_SOK)
    {
        status = EthFw_initMcm();
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to init CPSW MCM");
        EnetAppUtils_assert(status == ENET_SOK);
    }

    /* Initialize multicast support */
    if (status == ENET_SOK)
    {
        status = EthFwMcast_init(&config->mcastCfg,
                                 gEthFwObj.switchPortMask,
                                 gEthFwObj.macOnlyPortMask);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Incorrect shared mcast configuration");
    }

#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
    /* Initialize lwIP ARP helper */
    if (status == ENET_SOK)
    {
        status = EthFwArp_init();
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to init ARP utils");
    }
#endif

#if defined(ETHFW_VEPA_SUPPORT)
    status = EthFwVepa_init(&config->vepaCfg);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to init VEPA utils");
#endif

#if defined(ETHFW_IET_ENABLE)
    EthFwIET_init(&config->ietCfg, gEthFwObj.hEnet, gEthFwObj.coreId);
#endif

#if defined(ETHFW_MONITOR_SUPPORT)
    status = EthFwMon_init(&config->monitorCfg, gEthFwObj.enetType, gEthFwObj.instId, gEthFwObj.numPorts);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to init Monitor");
#endif

    /* Add ALE entry for broadcast MAC address. Note this is needed as the broadcast
     * is disabled via unknownRegMcastFloodMask and other flags in ALE init config.
     * In EthFw we need broadcast to handle ARP entries for clients */
    if (status == ENET_SOK)
    {
        status = EthFw_setAleBcastEntry();
    }

    /* Port mirroring */
    if (status == ENET_SOK)
    {
        status = EthFwPortMirroring_init(gEthFwObj.hEnet, config->portMirCfg);
    }

    /* Setup static VLANs */
    if (status == ENET_SOK)
    {
        status = EthFw_setupVlan(config);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to setup static VLANs");
    }

    /* As P0_RX_DSCP_MAP registers are not set to identity mapping by default
     * we are calling ENET_HOSTPORT_IOCTL_SET_INGRESS_DSCP_PRI_MAP to set the same.
     * It also sets P0_CONTROL_REG_DSCP_IPV4_EN and P0_CONTROL_REG_DSCP_IPV6_EN.
     * Note that P0_RX_PRI_MAP (for VLAN) is set to identity mapping by default.
     * Only P0_CONTROL_REG_RX_REMAP_VLAN needs to be set for vlan tagged packets */
    if (status == ENET_SOK)
    {
        status = EthFw_setDscpPriorityMapRegisters();
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to set DSCP priority mapping");
    }

    /* Passing error packets to host port is currently not supported on
     * CPSW instances in Main domain, due to potential HW limitation */
    gEthFwObj.passErrPkt = config->passErrPkt;
    if (gEthFwObj.passErrPkt == BTRUE)
    {
        ETHFWTRACE_WARN("Disabling error packet inspection feature due to HW limitation");
        gEthFwObj.passErrPkt = BFALSE;
    }

#if defined(ETHFW_GPTP_SUPPORT)
    /* Initializes tsn stack before calling gPTP task */
    tsnCfg.enetType       = gEthFwObj.enetType;
    tsnCfg.instId         = gEthFwObj.instId;
    tsnCfg.configPtpCb    = config->configPtpCb;
    tsnCfg.configPtpCbArg = config->configPtpCbArg;
    tsnCfg.ppsConfig      = config->ppsConfig;
    if (status == ENET_SOK)
    {
        EthFwTsn_init(&tsnCfg);
    }
#endif

#if defined(ETHFW_MONITOR_SUPPORT)
    /* Start the Monitor Task */
    if (status == ENET_SOK)
    {
        status = EthFwMon_startTask();
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to start monitor task");
    }
#endif

    return (status == ENET_SOK) ? &gEthFwObj : NULL;
}

void EthFw_deinit(EthFw_Handle hEthFw)
{
    uint32_t coreIdx;
    uint32_t i;

    EnetAppUtils_assert(hEthFw != NULL);

#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
    /* De-initialize lwIP ARP helper */
    EthFwArp_deinit();
#endif

#if defined(ETHFW_VEPA_SUPPORT)
    /* De-initialize VEPA table */
    EthFwVepa_deinit();
#endif

    EthFwMcast_deinit();

    EthFwVlan_deinit(gEthFwObj.hEnet);

#if defined(ETHFW_MONITOR_SUPPORT)
    /* Stop the Monitor Task */
    EthFwMon_stopTask();
#endif

#if defined(ETHFW_GPTP_SUPPORT)
    EthFwTsn_deInit();
#endif

    /* De-initialize MCM */
    EthFw_deinitMcm();

    gEthFwObj.numPorts = 0U;

    /* Un-initialize EthFw status */
    EthFw_setStatus(ETHREMOTECFG_SERVERSTATUS_UNINIT);

    memset(&gEthFwObj.cpswCfg, 0, sizeof(Cpsw_Cfg));

    for ( i = 0U; i < gEthFwObj.numVirtPorts; i++)
    {
        coreIdx = EthFw_getCoreDmaResIndex(gEthFwObj.virtPortCfg[i].remoteCoreId);
        gEthFw_rmResPrms.coreResInfo[coreIdx].numTxCh       = 0U;
        gEthFw_rmResPrms.coreResInfo[coreIdx].numRxFlows    = 0U;
        gEthFw_rmResPrms.coreResInfo[coreIdx].numMacAddress = 0U;
    }
}

uint32_t EthFw_getRemoteEndptId(uint32_t coreId)
{
    uint32_t i;
    bool foundCoreId = BFALSE;
    uint32_t remoteEndptId = ETHFW_IPC_INVALID_RPMSG_ENDPOINT;

    /* match the coreID the number of remote_device-based virtual ports */
    for (i = 0U; i < ENET_ARRAYSIZE(gEthFw_autosarEndptId); i++)
    {
        if(coreId == gEthFw_autosarEndptId[i].remoteCoreId)
        {
            foundCoreId = BTRUE;
            remoteEndptId = gEthFw_autosarEndptId[i].remoteEndptId;
            break;
        }
    }
    EnetAppUtils_assert(BTRUE == foundCoreId);
    return remoteEndptId;
}

int32_t EthFw_initRemoteConfig(EthFw_Handle hEthFw)
{
    CpswProxyServer_Config_t cfg;
    int32_t status = ETHFW_EFAIL;
    uint32_t i;
    uint32_t numAutosarVirtPorts = 0U;
    uint32_t j;
    uint32_t k;

    EnetAppUtils_assert(hEthFw != NULL);

    /* Initialize Proxy Server */
    memset(&cfg, 0, sizeof(cfg));
    cfg.enetType = gEthFwObj.enetType;
    cfg.instId = gEthFwObj.instId;
    cfg.getMcmCmdIfCb = &EthFw_getMcmCmdIfCb;
    cfg.initEthfwDeviceDataCb = &EthFw_getDeviceData;
    cfg.notifyCb = &EthFw_handleProfileInfoNotify;

    ETHFW_UTILS_COMPILETIME_ASSERT(CPSWPROXYSERVER_MAC_PORT_MAX == ETHFW_MAC_PORT_MAX);

    /* Remote cores which use remote_device framework */
    for (i = 0U; i < gEthFwObj.numVirtPorts; i++)
    {
        cfg.virtPortCfg[i].clientIdMask = gEthFwObj.virtPortCfg[i].clientIdMask;
        /* If this virtual port is used by Autosar */
        if (ETHFW_IS_BIT_SET(cfg.virtPortCfg[i].clientIdMask, ETHREMOTECFG_CLIENTID_AUTOSAR))
        {
            cfg.virtPortCfg[i].endPointId = EthFw_getRemoteEndptId(gEthFwObj.virtPortCfg[i].remoteCoreId);
            numAutosarVirtPorts++;
        }
        
        cfg.virtPortCfg[i].remoteCoreId  = gEthFwObj.virtPortCfg[i].remoteCoreId;
        cfg.virtPortCfg[i].portId        = gEthFwObj.virtPortCfg[i].portId;
        cfg.virtPortCfg[i].numTxCh       = gEthFwObj.virtPortCfg[i].numTxCh;
        cfg.virtPortCfg[i].numRxFlow     = gEthFwObj.virtPortCfg[i].numRxFlow;
        cfg.virtPortCfg[i].numMacAddress = gEthFwObj.virtPortCfg[i].numMacAddress;
        for (j = 0U; j < cfg.virtPortCfg[i].numTxCh; j++)
        {
            cfg.virtPortCfg[i].txCh[j]   = gEthFwObj.virtPortCfg[i].txCh[j];
        }
        /* Storing each rx flow information for a virtual port */
        for (j = 0U; j < cfg.virtPortCfg[i].numRxFlow; j++)
        {
            cfg.virtPortCfg[i].rxFlowsInfo[j].numCustomPolicers = gEthFwObj.virtPortCfg[i].rxFlowsInfo[j].numCustomPolicers;
            /* Copying custom policers for each rx flow for a virtual port */
            if (cfg.virtPortCfg[i].rxFlowsInfo[j].numCustomPolicers != 0U)
            {
                for (k = 0U; k < cfg.virtPortCfg[i].rxFlowsInfo[j].numCustomPolicers; k++)
                {
                    cfg.virtPortCfg[i].rxFlowsInfo[j].customPolicersInArgs[k] = gEthFwObj.virtPortCfg[i].rxFlowsInfo[j].customPolicersInArgs[k];
                }
            }
        }
    }
    cfg.numVirtPorts = gEthFwObj.numVirtPorts;

    cfg.dfltVlanIdMacOnlyPorts = gEthFwObj.dfltVlanIdMacOnlyPorts;
    cfg.dfltVlanIdSwitchPorts  = gEthFwObj.dfltVlanIdSwitchPorts;

    cfg.enabledPortMask = gEthFwObj.enabledPortMask;
    cfg.macOnlyPortMask = gEthFwObj.macOnlyPortMask;

    status = CpswProxyServer_init(&cfg);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to init CPSW Proxy");

    /* CPSW proxy server is initialized, update the EthFw status. */
    if (status == ENET_SOK)
    {
        EthFw_setStatus(ETHREMOTECFG_SERVERSTATUS_READY);
    }

    return status;
}

int32_t EthFw_lateAnnounce(EthFw_Handle hEthFw,
                           uint32_t procId)
{
    int32_t status;

    EnetAppUtils_assert(hEthFw != NULL);

    /* Late announcement of server's endpoint to remote processor */
    status = CpswProxyServer_lateAnnounce(procId);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Late announcement to proc %u failed", procId);

    return status;
}

void EthFw_getVersion(EthFw_Handle hEthFw,
                      EthFw_Version *version)
{
    EnetAppUtils_assert(hEthFw != NULL);

    *version = gEthFwObj.version;
}

EthRemoteCfg_ServerStatus EthFw_getStatus(void)
{
    return gEthFwObj.serverStatus;
}

void EthFw_setStatus(EthRemoteCfg_ServerStatus serverStatus)
{
    gEthFwObj.serverStatus = serverStatus;
}

static int32_t EthFw_initMcm(void)
{
    EnetMcm_InitConfig mcmCfg;
    EnetMcm_HandleInfo handleInfo;
    uint32_t i;
    int32_t status = ENET_SOK;

    /* Initialize CPSW MCM */
    mcmCfg.perCfg = (void *)&gEthFwObj.cpswCfg;
    mcmCfg.enetType = gEthFwObj.enetType;
    mcmCfg.instId = gEthFwObj.instId;
    mcmCfg.setPortLinkCfg = EthFw_initLinkArgs;
    mcmCfg.numMacPorts = gEthFwObj.numPorts;
    mcmCfg.periodicTaskPeriod = ENETPHY_FSM_TICK_PERIOD_MS;
    mcmCfg.print = EthFwTrace_print;
#if !defined(MCU_PLUS_SDK)
    mcmCfg.traceTsFunc = NULL;
    mcmCfg.extTraceFunc = NULL;
#endif

    for (i = 0U; i < gEthFwObj.numPorts; i++)
    {
        mcmCfg.macPortList[i] = gEthFwObj.ports[i].macPort;
    }

    if ((mcmCfg.enetType != ENET_CPSW_5G) &&
        (mcmCfg.enetType != ENET_CPSW_9G) &&
        (mcmCfg.enetType != ENET_CPSW_3G))
    {
        status = ENET_ENOTSUPPORTED;
    }

    if (status == ENET_SOK)
    {
        status = EnetMcm_init(&mcmCfg);
        EnetAppUtils_assert(status == ENET_SOK);
    }

    /* Get MCM command interface */
    if (status == ENET_SOK)
    {
        EnetMcm_getCmdIf(gEthFwObj.enetType, &gEthFwObj.mcmCmdIf);
        EnetAppUtils_assert(gEthFwObj.mcmCmdIf.hMboxCmd != NULL);
        EnetAppUtils_assert(gEthFwObj.mcmCmdIf.hMboxResponse != NULL);
    }

    /* Get MCM handle - CPSW driver should be open as a consequence */
    if (status == ENET_SOK)
    {
        EnetMcm_acquireHandleInfo(&gEthFwObj.mcmCmdIf, &handleInfo);
        gEthFwObj.hEnet = handleInfo.hEnet;
    }

    return status;
}

static void EthFw_deinitMcm(void)
{
    /* Release MCM handle - CPSW should close if we're last client */
    EnetMcm_releaseHandleInfo(&gEthFwObj.mcmCmdIf);
    EnetMcm_releaseCmdIf(gEthFwObj.enetType, &gEthFwObj.mcmCmdIf);

    /* De-initialize CPSW MCM */
    EnetMcm_deInit(gEthFwObj.enetType);
}

static void EthFw_initLinkArgs(EnetPer_PortLinkCfg *linkArgs,
                               Enet_MacPort macPort)
{
    EnetPhy_Cfg *phyCfg = &linkArgs->phyCfg;
    CpswMacPort_Cfg *macCfg = (CpswMacPort_Cfg *)linkArgs->macCfg;
    EnetMacPort_LinkCfg *linkCfg = &linkArgs->linkCfg;
    EnetMacPort_Interface *mii = &linkArgs->mii;
    uint32_t i;
    int32_t status;

    /* Port link config is set by app */
    status = gEthFwObj.setPortCfg(macPort, macCfg, mii, phyCfg, linkCfg);
    if (status != ENET_SOK)
    {
        ETHFWTRACE_ERR(status, "Failed to set MAC port %u config", ENET_MACPORT_ID(macPort));
        EnetAppUtils_assert(BFALSE);
    }

    for (i = 0U; i < gEthFwObj.numPorts; i++)
    {
        if (gEthFwObj.ports[i].macPort == macPort)
        {
            macCfg->vlanCfg = gEthFwObj.ports[i].vlanCfg;
        }
    }

#if !defined(MCU_PLUS_SDK)
    /* Pass MAC control, short or error packets to host port */
    if (gEthFwObj.passErrPkt)
    {
        macCfg->rxCmfEn = BTRUE;
        macCfg->rxCsfEn = BTRUE;
        macCfg->rxCefEn = BTRUE;
    }
#endif
}

static int32_t EthFw_setAleBcastEntry(void)
{
    Enet_IoctlPrms prms;
    uint32_t setMcastOutArgs;
    CpswAle_SetMcastEntryInArgs setMcastInArgs;
    uint8_t bCastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    int32_t status;

    memcpy(&setMcastInArgs.addr.addr[0], &bCastAddr[0U], sizeof(setMcastInArgs.addr.addr));
    setMcastInArgs.addr.vlanId     = 0U;
    setMcastInArgs.info.super      = BFALSE;
    setMcastInArgs.info.fwdState   = CPSW_ALE_FWDSTLVL_FWD;
    setMcastInArgs.info.portMask   = CPSW_ALE_ALL_PORTS_MASK;
    setMcastInArgs.info.numIgnBits = 0U;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &setMcastInArgs, &setMcastOutArgs);

    ENET_IOCTL(gEthFwObj.hEnet, gEthFwObj.coreId, CPSW_ALE_IOCTL_ADD_MCAST, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add bcast ALE entry");

    return status;
}

/* Application callback to get MCM command interface */

static void EthFw_getMcmCmdIfCb(Enet_Type enetType,
                                EnetMcm_CmdIf **pMcmCmdIfHandle)
{
    *pMcmCmdIfHandle = &gEthFwObj.mcmCmdIf;
}

static void EthFw_getDeviceData(EthRemoteCfg_DeviceData *ethdevData)
{
    ethdevData->fwVer.major = gEthFwObj.version.major;
    ethdevData->fwVer.minor = gEthFwObj.version.minor;
    ethdevData->fwVer.rev = gEthFwObj.version.rev;

    memcpy(ethdevData->fwVer.month,
           &gEthFwObj.version.month[0U],
           sizeof(ethdevData->fwVer.month));

    memcpy(ethdevData->fwVer.date,
           &gEthFwObj.version.date[0U],
           sizeof(ethdevData->fwVer.date));

    memcpy(ethdevData->fwVer.year,
           &gEthFwObj.version.year[0U],
           sizeof(ethdevData->fwVer.year));

    memcpy(ethdevData->fwVer.commitHash,
           &gEthFwObj.version.commitHash[0U],
           sizeof(ethdevData->fwVer.commitHash));

    /* Enable permission for all ETHDEV remote commands without consideration of cores.
     * This should be changed based on trusted cores */
    ethdevData->permissionFlags = ((1ULL << ETHREMOTECFG_CMD_TYPE_LAST) - 1);
    ethdevData->uartConnected = BTRUE;
    ethdevData->uartId = ENET_UTILS_MCU2_0_UART_INSTANCE;
}

static void EthFw_handleProfileInfoNotify(uint32_t hostId,
                                          Enet_Handle hEnet,
                                          Enet_Type enetType,
                                          uint32_t coreKey,
                                          EthRemoteCfg_NotifyType notifyid,
                                          uint8_t *notifyInfo,
                                          uint32_t notifyInfoLen)
{
    /* Nothing to do */
}
