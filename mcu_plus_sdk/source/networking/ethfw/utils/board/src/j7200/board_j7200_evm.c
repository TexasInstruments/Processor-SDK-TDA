/*
 *  Copyright (c) Texas Instruments Incorporated 2022-2025
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file  board_j7200_evm.c
 *
 * \brief This file contains the implementation of the J7200 EVM board
 *        configuration functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x403

#include <ti/drv/enet/enet.h>
#include <ti/drv/enet/examples/utils/include/enet_apputils.h>

#include <ti/board/board.h>
#include <ti/board/src/j7200_evm/include/board_cfg.h>
#include <ti/board/src/j7200_evm/include/board_pinmux.h>
#include <ti/board/src/j7200_evm/include/board_utils.h>
#include <ti/board/src/j7200_evm/include/board_control.h>
#include <ti/board/src/j7200_evm/include/board_ethernet_config.h>
#include <ti/board/src/j7200_evm/include/board_serdes_cfg.h>

#include <utils/board/include/ethfw_board_utils.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* UART port domain/instance used for logging */
#define ETHFW_UART_DOMAIN                       (BOARD_SOC_DOMAIN_MAIN)
#define ETHFW_UART_INST                         (3U)

/* CPTS clock source */
#define ETHFW_CPSW_CPTS_RFT_CLK                 (ENET_CPSW0_CPTS_CLKSEL_MAIN_SYSCLK0)

/* VSC8514 wait time between NRESET deassert and access of the SMI interface */
#define ETHFW_QSGMII_PHY_TWAIT_MSECS            (105U)

/* Maximum number of physical Ethernet ports supported for this board */
#define ETHFW_MAX_STATIC_MAC_ADDR               (9U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* EthFw board configuration params object */
typedef struct EthFwBoard_Obj_s
{
    /* UART configuration allowed */
    bool uartAllowed;

    /* I2C configuration allowed */
    bool i2cAllowed;

    /* SerDes configuration allowed */
    bool serdesAllowed;

    /* QSGMII board to be enabled or not */
    bool qenetEnabled;

    /* QSGMII board detected */
    bool qenetDetected;

    /* ENET bridge board to be enabled or not */
    bool enetBridgeEnabled;
} EthFwBoard_Obj;

/*!
 * \brief Board related configuration parameters of an Ethernet PHY.
 */
typedef struct EthFwBoard_PhyCfg_s
{
    /*! PHY device address */
    uint32_t phyAddr;

    /*! Interface type */
    EnetPhy_Mii mii;

    /*! Whether PHY is strapped or not */
    bool isStrapped;

    /*! Whether to skip PHY-specific extended configuration */
    bool skipExtendedCfg;

    /*! Extended PHY-specific configuration */
    const void *extendedCfg;

    /*! Size of the extended configuration */
    uint32_t extendedCfgSize;
} EthFwBoard_PhyCfg;

/*!
 * \brief Ethernet port configuration parameters.
 */
typedef struct EthFwBoard_MacPortCfg_s
{
    /*! MAC port connected to */
    Enet_MacPort macPort;

    /*! MAC port interface */
    EnetMacPort_Interface mii;

    /*! PHY configuration parameters */
    EthFwBoard_PhyCfg phyCfg;

    /*! SGMII mode. Applicable only when port is used in Q/SGMII mode */
    EnetMac_SgmiiMode sgmiiMode;

    /*! Link configuration (speed and duplexity) */
    EnetMacPort_LinkCfg linkCfg;
} EthFwBoard_MacPortCfg;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void EthFwBoard_enableMods(void);

static void EthFwBoard_configPinmux(void);

static void EthFwBoard_detectDBs(void);

static void EthFwBoard_configUart(void);

static void EthFwBoard_configQenet(void);

static void EthFwBoard_configSerdesBridge(void);

static void EthFwBoard_configTorrentClks(void);

static void EthFwBoard_configCpswClocks(void);

static uint32_t EthFwBoard_getMacAddrPoolEeprom(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                                uint32_t poolSize);

static uint32_t EthFwBoard_getMacAddrPoolStatic(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                                uint32_t poolSize);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EthFwBoard_Obj gEthFwBoard;

#if defined(ETHFW_PPS_DEMO_SUPPORT)
extern pinmuxBoardCfg_t gEthFwPinmuxData[];
#endif

/* Default port configuration for all ports in EVM:
 *   4 x QSGMII ports in QEnet */
static EthFwBoard_MacPortCfg gEthFw_qenetMacPortCfg[] =
{
    {   /* "P0" */
        .macPort   = ENET_MAC_PORT_1,
        .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_QUAD_SERIAL_MAIN },
        .phyCfg    =
        {
            .phyAddr         = 16U,
            .isStrapped      = BFALSE,
            .skipExtendedCfg = BFALSE,
            .extendedCfg     = NULL,
            .extendedCfgSize = 0U,
        },
        .sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_WITH_PHY,
        .linkCfg   = { ENET_SPEED_AUTO, ENET_DUPLEX_AUTO },
    },
    {   /* "P1" */
        .macPort   = ENET_MAC_PORT_2,
        .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_QUAD_SERIAL_SUB },
        .phyCfg    =
        {
            .phyAddr         = 17U,
            .isStrapped      = BFALSE,
            .skipExtendedCfg = BFALSE,
            .extendedCfg     = NULL,
            .extendedCfgSize = 0U,
        },
        .sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_WITH_PHY,
        .linkCfg   = { ENET_SPEED_AUTO, ENET_DUPLEX_AUTO },
    },
    {   /* "P2" */
        .macPort   = ENET_MAC_PORT_3,
        .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_QUAD_SERIAL_SUB },
        .phyCfg    =
        {
            .phyAddr         = 18U,
            .isStrapped      = BFALSE,
            .skipExtendedCfg = BFALSE,
            .extendedCfg     = NULL,
            .extendedCfgSize = 0U,
        },
        .sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_WITH_PHY,
        .linkCfg   = { ENET_SPEED_AUTO, ENET_DUPLEX_AUTO },
    },
    {   /* "P3" */
        .macPort   = ENET_MAC_PORT_4,
        .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_QUAD_SERIAL_SUB },
        .phyCfg    =
        {
            .phyAddr         = 19U,
            .isStrapped      = BFALSE,
            .skipExtendedCfg = BFALSE,
            .extendedCfg     = NULL,
            .extendedCfgSize = 0U,
        },
        .sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_WITH_PHY,
        .linkCfg   = { ENET_SPEED_AUTO, ENET_DUPLEX_AUTO },
    },
};

/* 1 x SGMII port in MAC-to-MAC mode using (SGMII) ENET bridge expansion board */
static EthFwBoard_MacPortCfg gEthFw_enetBridgeMacPortCfg =
{
    .macPort   = ENET_MAC_PORT_1,
    .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_SERIAL },
    .phyCfg    =
    {
        .phyAddr         = ENETPHY_INVALID_PHYADDR,
        .isStrapped      = BFALSE,
        .skipExtendedCfg = BFALSE,
        .extendedCfg     = NULL,
        .extendedCfgSize = 0U,
    },
    .sgmiiMode = ENET_MAC_SGMIIMODE_SGMII_FORCEDLINK,
    .linkCfg   = { ENET_SPEED_1GBIT, ENET_DUPLEX_FULL },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwBoard_init(uint32_t flags)
{
    Board_initCfg boardCfg = 0U;
    Board_STATUS boardStatus;

    /* Save the functionality requested by app */
    gEthFwBoard.enetBridgeEnabled = ENET_NOT_ZERO(flags & ETHFW_BOARD_ENET_BRIDGE_ENABLE);
    gEthFwBoard.qenetEnabled  = ENET_NOT_ZERO(flags & ETHFW_BOARD_QENET_ENABLE);
    gEthFwBoard.serdesAllowed = ENET_NOT_ZERO(flags & ETHFW_BOARD_SERDES_CONFIG);
    gEthFwBoard.uartAllowed   = ENET_NOT_ZERO(flags & ETHFW_BOARD_UART_ALLOWED);
    gEthFwBoard.i2cAllowed    = ENET_NOT_ZERO(flags & ETHFW_BOARD_I2C_ALLOWED);

    /* Enable hardware block/modules that EthFw will need */
    EthFwBoard_enableMods();

    /* Configure pinmux only for the pins used by EthFw */
    EthFwBoard_configPinmux();

    /* Configure UART used for EthFw logging */
    if (gEthFwBoard.uartAllowed)
    {
        EthFwBoard_configUart();
        boardCfg |= BOARD_INIT_UART_STDIO;
    }

    /* Initialize board via board library */
    boardCfg |= BOARD_INIT_ENETCTRL_CPSW5G;
    boardStatus = Board_init(boardCfg);
    ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                      "Failed to initialize board");
    EnetAppUtils_assert(boardStatus == BOARD_SOK);

    /* Detect expansion boards attached to main board (CPB) */
    EthFwBoard_detectDBs();

    /* Configure QSGMII expansion board */
    if (gEthFwBoard.qenetEnabled)
    {
        EthFwBoard_configQenet();
    }

    /* Configure SerDes for SGMII bridge */
    if (gEthFwBoard.enetBridgeEnabled)
    {
        EthFwBoard_configSerdesBridge();
    }

    /* Set CPSW clocks: CPPI, RGMII 5/50/250 MHz, CPTS */
    EthFwBoard_configCpswClocks();

    return boardStatus;
}

int32_t EthFwBoard_validateMacPorts(Enet_MacPort* enabledMacPortList,
                                    uint8_t numEnabledMacPortList,
                                    Enet_MacPort* gptpEnabledPortList,
                                    uint8_t numGptpEnabledPortList
                                    )
{
    uint8_t gesiPortIndex = 0U;
    uint8_t enabledPortIndex = 0U;
    uint8_t gptpEnabledPortIndex = 0U;
    uint32_t enabledMacPortMask = 0U;
    uint32_t gptpEnabledPortMask = 0U;
    int32_t status = BOARD_SOK;

    /* Calculating port mask for the ports enabled by EthFw. */
    for(enabledPortIndex = 0; enabledPortIndex < numEnabledMacPortList; enabledPortIndex++)
    {
        enabledMacPortMask |= ENET_MACPORT_MASK(enabledMacPortList[enabledPortIndex]);
    }

    if ((gptpEnabledPortList != NULL) && (status == BOARD_SOK))
    {
        for (gptpEnabledPortIndex=0; gptpEnabledPortIndex < numGptpEnabledPortList; gptpEnabledPortIndex++)
        {
            gptpEnabledPortMask |= ENET_MACPORT_MASK(gptpEnabledPortList[gptpEnabledPortIndex]);
        }

        if ((gptpEnabledPortMask & enabledMacPortMask) != gptpEnabledPortMask)
        {
            status = BOARD_INVALID_PARAM;
            ETHFWTRACE_ERR_IF((status != BOARD_SOK),
                               status,
                               "MAC ports required for gPTP support are not enabled.");
        }
    }

    return status;
}

uint32_t EthFwBoard_getMacPorts(Enet_MacPort macPorts[ENET_MAC_PORT_NUM])
{
    uint32_t num = 0U;
    uint32_t req;
    uint32_t i;

    memset(macPorts, 0, sizeof(*macPorts));

    if (gEthFwBoard.qenetEnabled)
    {
        req = EnetUtils_min(ENET_MAC_PORT_NUM, ENET_ARRAYSIZE(gEthFw_qenetMacPortCfg));
        for (i = 0U; i < req; i++)
        {
            macPorts[num++] = gEthFw_qenetMacPortCfg[i].macPort;
        }
    }

    if (gEthFwBoard.enetBridgeEnabled)
    {
        if (num < ENET_MAC_PORT_NUM)
        {
            macPorts[num++] = gEthFw_enetBridgeMacPortCfg.macPort;
        }
    }

    return num;
}

static const EthFwBoard_MacPortCfg *EthFwBoard_findPortCfg(Enet_MacPort macPort)
{
    const EthFwBoard_MacPortCfg *portCfg = NULL;
    uint32_t i;

    if (gEthFwBoard.qenetEnabled)
    {
        for (i = 0U; i < ENET_ARRAYSIZE(gEthFw_qenetMacPortCfg); i++)
        {
            if (gEthFw_qenetMacPortCfg[i].macPort == macPort)
            {
                portCfg = &gEthFw_qenetMacPortCfg[i];
                break;
            }
        }
    }

    if ((portCfg == NULL) && gEthFwBoard.enetBridgeEnabled)
    {
        if (gEthFw_enetBridgeMacPortCfg.macPort == macPort)
        {
            portCfg = &gEthFw_enetBridgeMacPortCfg;
        }
    }

    return portCfg;
}

int32_t EthFwBoard_setPortCfg(Enet_MacPort macPort,
                              CpswMacPort_Cfg *macCfg,
                              EnetMacPort_Interface *mii,
                              EnetPhy_Cfg *phyCfg,
                              EnetMacPort_LinkCfg *linkCfg)
{
    const EthFwBoard_MacPortCfg *portCfg;
    int32_t status = ENET_ENOTFOUND;

    CpswMacPort_initCfg(macCfg);
    EnetPhy_initCfg(phyCfg);

    portCfg = EthFwBoard_findPortCfg(macPort);
    if (portCfg != NULL)
    {
        /* Set MII configuration: RGMII or Q/SGMII */
        *mii = portCfg->mii;
        mii->variantType = ENET_MAC_VARIANT_FORCED;

        /* Set PHY configuration parameters */
        phyCfg->phyAddr         = portCfg->phyCfg.phyAddr;
        phyCfg->isStrapped      = portCfg->phyCfg.isStrapped;
        phyCfg->loopbackEn      = BFALSE;
        phyCfg->skipExtendedCfg = portCfg->phyCfg.skipExtendedCfg;
        phyCfg->extendedCfgSize = portCfg->phyCfg.extendedCfgSize;
        memcpy(phyCfg->extendedCfg, portCfg->phyCfg.extendedCfg, portCfg->phyCfg.extendedCfgSize);

        /* Set link configuration: speed and duplex */
        *linkCfg = portCfg->linkCfg;

        /* Set SGMII mode (applicable for Q/SGMII ports only) */
        macCfg->sgmiiMode = portCfg->sgmiiMode;

        /* Set MAC port RX MTU (MRU) to incorporate VLAN tagged packets */
        macCfg->rxMtu = 1522U;

        status = ENET_SOK;
    }
    else
    {
        ETHFWTRACE_ERR(status, "Port %u params not found", ENET_MACPORT_ID(macPort));
    }

    return status;
}

static void EthFwBoard_enableMods(void)
{
    uint32_t i;
    uint32_t clkModId[] = { TISCI_DEV_DDR0,
                            TISCI_DEV_TIMER12,
                            TISCI_DEV_TIMER13,
                            TISCI_DEV_UART3, };

    /* Enable hardware block/modules that EthFw will need */
    for (i = 0U; i < ENET_ARRAYSIZE(clkModId); i++)
    {
        EnetAppUtils_setDeviceState(clkModId[i], TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0U);
    }
}

static void EthFwBoard_configPinmux(void)
{
    Board_PinmuxConfig_t pinmuxCfg;

    /* Pinmux configuration of pins used for I2C, PHY reset gpios, UART, etc. */
    Board_pinmuxUpdate(gJ7200_MainPinmuxDataCpsw, BOARD_SOC_DOMAIN_MAIN);

    /* Configure pinmux settings for Ethernet ports and MDIO */
    Board_pinmuxUpdate(gJ7200_WkupPinmuxDataCpsw, BOARD_SOC_DOMAIN_MAIN);

#if defined(ETHFW_PPS_DEMO_SUPPORT)
    /* Configure pinmux settings for Ethernet ports and MDIO */
    Board_pinmuxUpdate(gEthFwPinmuxData, BOARD_SOC_DOMAIN_MAIN);
#endif
}

static void EthFwBoard_detectDBs(void)
{
    gEthFwBoard.qenetDetected = Board_detectBoard(BOARD_ID_ENET);
    ETHFWTRACE_INFO("Detected boards:%s", gEthFwBoard.qenetDetected ? " QSGMII" : "");
}

static void EthFwBoard_configUart(void)
{
    Board_initParams_t initParams;
    Board_STATUS boardStatus;

    Board_getInitParams(&initParams);
    initParams.uartSocDomain = ETHFW_UART_DOMAIN;
    initParams.uartInst      = ETHFW_UART_INST;
    initParams.pscMode       = BOARD_PSC_DEVICE_MODE_NONEXCLUSIVE;
    Board_setInitParams(&initParams);

    if (gEthFwBoard.i2cAllowed)
    {
        /* Set SOM's MUX2 for UART3 route */
        boardStatus = Board_control(BOARD_CTRL_CMD_SET_SOM_UART_MUX, NULL);
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to set SOM board's MUX2");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
    }
}

static void EthFwBoard_configQenet(void)
{
    Board_STATUS boardStatus;

    if (gEthFwBoard.i2cAllowed)
    {
        /* Release PHY reset */
        boardStatus = Board_cpswEnetExpPhyReset(0U);
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to release QSGMII PHY out of reset");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);

        /* Release the COMA_MODE pin */
        boardStatus = Board_cpswEnetExpComaModeCfg(0U);
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to release QSGMII PHY out of reset");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
    }

    /* Wait till we can access QSGMII PHY registers after reset, irrespective
     * of the NRESET gpio set by ETHFW or bootloader */
    EthFwOsal_sleepTaskinMsecs(ETHFW_QSGMII_PHY_TWAIT_MSECS);

    if (gEthFwBoard.serdesAllowed)
    {
        /* Configure SerDes clocks */
        EthFwBoard_configTorrentClks();

        /* Configure SerDes for QSGMII functionality */
        boardStatus = Board_serdesCfgQsgmii();
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to configure SerDes for QSGMII");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
    }
}

static void EthFwBoard_configSerdesBridge(void)
{
    Board_STATUS boardStatus;

    if (gEthFwBoard.serdesAllowed)
    {
        /* Configure SerDes clocks */
        EthFwBoard_configTorrentClks();

        /* Configure SerDes for SGMII functionality */
        boardStatus = Board_serdesCfgSgmii();
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to configure SerDes for SGMII");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);

        /* Set MAC mode to SGMII */
        boardStatus = Board_cpsw5gEthConfig(ENET_MACPORT_NORM(ENET_MAC_PORT_1), SGMII);
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to set SoC MAC mode");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
    }
}

static void EthFwBoard_configTorrentClks(void)
{
    uint32_t moduleId = TISCI_DEV_SERDES_10G1;
    uint32_t clkParId = TISCI_DEV_SERDES_10G1_CORE_REF_CLK_PARENT_HSDIV4_16FFT_MAIN_2_HSDIVOUT4_CLK;
    uint32_t clkId = TISCI_DEV_SERDES_10G1_CORE_REF_CLK;
    uint32_t clkRateHz = 100000000U;
    uint32_t origParent = 0U;
    int32_t status;

    status = Sciclient_pmGetModuleClkParent(moduleId, clkId, &origParent, SCICLIENT_SERVICE_WAIT_FOREVER);

    /* Read the clock and set it to clkParId only if it is different. */
    if ((status == CSL_PASS) && (origParent != clkParId ))
    {
        status = Sciclient_pmSetModuleClkParent(moduleId, clkId, clkParId, SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status != CSL_PASS)
        {
            ETHFWTRACE_ERR(status, "Failed to reparent clk %u", clkId);
            EnetAppUtils_assert(BFALSE);
        }
    }

    EnetAppUtils_clkRateSet(moduleId, clkId, clkRateHz);

    EnetAppUtils_setDeviceState(moduleId, TISCI_MSG_VALUE_DEVICE_SW_STATE_ON, 0U);
}

static void EthFwBoard_configCpswClocks(void)
{
    /* Enable CPPI_CLK_CLK and set RGMII_MHZ_[5,50,250]_CLK clock rate */
    EnetAppUtils_enableClocks(ENET_CPSW_5G, 0U);

    /* Select CPTS source clock (CPTS_RFT_CLK) in CLKSEL mux */
    EnetAppUtils_selectCptsClock(ENET_CPSW_5G, 0U, ETHFW_CPSW_CPTS_RFT_CLK);
}

uint32_t EthFwBoard_getMacAddrPool(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                   uint32_t poolSize)
{
    uint32_t allocCnt = 0U;
    uint32_t staticCnt = 0U;

    if (gEthFwBoard.i2cAllowed)
    {
        allocCnt = EthFwBoard_getMacAddrPoolEeprom(macAddr, poolSize);
    }

    if (allocCnt < poolSize)
    {
        staticCnt = EthFwBoard_getMacAddrPoolStatic(&macAddr[allocCnt], poolSize - allocCnt);
        allocCnt += staticCnt;
    }

    return allocCnt;
}

static uint32_t EthFwBoard_getMacAddrPoolEeprom(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                                uint32_t poolSize)
{
    uint8_t macAddrBuf[ENET_RM_NUM_MACADDR_MAX * BOARD_MAC_ADDR_BYTES];
    Board_STATUS boardStatus;
    uint32_t macAddrCnt, tempCnt;
    uint32_t allocCnt = 0U;
    uint32_t i;

    if (gEthFwBoard.qenetEnabled && gEthFwBoard.qenetDetected)
    {
        /* Read number of MAC addresses in QUAD Eth board */
        boardStatus = Board_readMacAddrCount(BOARD_ID_ENET, &macAddrCnt);
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to read QpENet EEPROM");
        ETHFWTRACE_ERR_IF((macAddrCnt > ENET_RM_NUM_MACADDR_MAX), ETHFW_EINVALIDPARAMS,
                          "Exceeded max number of MAC addresses %u (max %u)",
                          macAddrCnt, ENET_RM_NUM_MACADDR_MAX);
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
        EnetAppUtils_assert(macAddrCnt <= ENET_RM_NUM_MACADDR_MAX);

        /* Read MAC addresses */
        boardStatus = Board_readMacAddr(BOARD_ID_ENET, macAddrBuf, sizeof(macAddrBuf), &tempCnt);
        ETHFWTRACE_ERR_IF((boardStatus != BOARD_SOK), boardStatus,
                          "Failed to read QpENet EEPROM");
        ETHFWTRACE_ERR_IF((tempCnt != macAddrCnt), ETHFW_EUNEXPECTED,
                          "Mismatching num of MAC addresses in QpENet EEPROM");
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
        EnetAppUtils_assert(tempCnt == macAddrCnt);

        /* Save only those required to meet the max number of MAC entries */
        allocCnt = EnetUtils_min(macAddrCnt, poolSize);
        for (i = 0U; i < allocCnt; i++)
        {
            memcpy(macAddr[i], &macAddrBuf[i * BOARD_MAC_ADDR_BYTES], ENET_MAC_ADDR_LEN);
        }

        if (allocCnt == 0U)
        {
            ETHFWTRACE_ERR(ENET_EALLOC, "No MAC addresses read from QENET board");
            EnetAppUtils_assert(BFALSE);
        }
    }

    return allocCnt;
}

static uint32_t EthFwBoard_getMacAddrPoolStatic(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                                uint32_t poolSize)
{
    uint32_t macAddrCnt;
    uint32_t allocCnt = 0U;

    /*
     * Workaround for EthFw/u-boot I2C conflicts:
     * EthFw reads MAC addresses from GESI and QUAD_ETH boards during EthFw
     * initialization which are stored in EEPROM memories and are read via
     * I2C.  These I2C accesses tend to occur around the same u-boot is also
     * performing I2C accesses, causing transactions to timeout or other
     * similar symptoms.
     *
     * I2C sharing is a known limitation but no current solution exists at
     * this time.  As a temporary workaround, EthFw will use fixed MAC
     * addresses in Linux builds. For RTOS build, MAC addresses will still
     * be read from EEPROM as such I2C contention isn't an problem.
     */
#if defined(ETHFW_RAND_MACADDR_GEN)
    uint8_t macAddrBuf[ETHFW_MAX_STATIC_MAC_ADDR][ENET_MAC_ADDR_LEN];
    uint32_t seed = CycleprofilerP_getTimeStamp();
    uint32_t i;
    uint32_t j;

    memset(&macAddrBuf, 0, ETHFW_MAX_STATIC_MAC_ADDR*ENET_MAC_ADDR_LEN);

    srand(seed);

    for (i = 0U; i < ETHFW_MAX_STATIC_MAC_ADDR; i++)
    {
        macAddrBuf[i][0] = 0x70U;

        for (j = 1U; j < ENET_MAC_ADDR_LEN; j++)
        {
            macAddrBuf[i][j] = (uint8_t)rand()%255;
        }
    }
#else
    uint8_t macAddrBuf[][ENET_MAC_ADDR_LEN] =
    {
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC1U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC2U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC3U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC4U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC5U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0X8BU, 0xC4U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0X8BU, 0xC5U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0X8BU, 0xC6U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0X8BU, 0xC7U },
    };
#endif

    macAddrCnt = ENET_ARRAYSIZE(macAddrBuf);

    /* Save only those required to meet the max number of MAC entries */
    allocCnt = EnetUtils_min(macAddrCnt, poolSize);
    memcpy(&macAddr[0U][0U], &macAddrBuf[0U][0U], allocCnt * ENET_MAC_ADDR_LEN);

#if defined(ETHFW_RAND_MACADDR_GEN)
    ETHFWTRACE_WARN_IF((allocCnt > 0U),
                       "Warning: Using %u random MAC address(es)", allocCnt);
#else
    ETHFWTRACE_WARN_IF((allocCnt > 0U),
                       "Warning: Using %u MAC address(es) from static pool", allocCnt);
#endif

    return allocCnt;
}
