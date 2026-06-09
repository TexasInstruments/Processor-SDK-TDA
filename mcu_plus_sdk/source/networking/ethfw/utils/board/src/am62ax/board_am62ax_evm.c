/*
 *  Copyright (c) Texas Instruments Incorporated 2024
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
 * \file  board_am62ax_evm.c
 *
 * \brief This file contains the implementation of the AM62AX SK-EVM board
 *        configuration functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x406

#include <stdint.h>
#include <stdarg.h>

#include <enet_apputils.h>
#include <enet_appsoc.h>
#include <dp83867.h>
#include <mod/cpsw_macport.h>
#include <utils/board/include/ethfw_board_utils.h>
#include <utils/ethfw_common/include/ethfw_trace.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

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

    /* GPIO configuration allowed *///ToDo: check this needs to be set
    bool gpioAllowed;
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

static void EthFwBoard_configCpswClocks(void);

static uint32_t EthFwBoard_getMacAddrPoolEeprom(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                                uint32_t poolSize);

static uint32_t EthFwBoard_getMacAddrPoolStatic(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                                uint32_t poolSize);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*!
 * \brief Common Processor Board (CPB) board's DP83867 PHY configuration.
 */

static const Dp83867_Cfg gEnetCpbBoard_dp83867PhyCfg =
{
/* The delay values are set based on trial and error and not tuned per port of the evm */
    .txClkShiftEn         = true,
    .rxClkShiftEn         = true,
    .txDelayInPs          = 250U,   /* 0.25 ns */
    .rxDelayInPs          = 2000U,  /* 2.00 ns */
    .txFifoDepth          = 4U,
    .impedanceInMilliOhms = 35000,  /* 35 ohms */
    .idleCntThresh        = 4U,     /* Improves short cable performance */
    .gpio0Mode            = DP83867_GPIO0_LED3,
    .gpio1Mode            = DP83867_GPIO1_COL, /* Unused */
    .ledMode              =
    {
        DP83867_LED_LINKED,         /* Unused */
        DP83867_LED_LINKED_100BTX,
        DP83867_LED_RXTXACT,
        DP83867_LED_LINKED_1000BT,
    },
};

/* 1 x SGMII port in MAC-to-MAC mode using (SGMII) ENET bridge expansion board */
static EthFwBoard_MacPortCfg gEthFw_cpbMacPortCfg[] =
{
    {   /* "CPSW3G" */
        .macPort   = ENET_MAC_PORT_1,
        .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg    =
        {
            .phyAddr         = 0U,
            .isStrapped      = BFALSE,
            .skipExtendedCfg = BFALSE,
            .extendedCfg     = &gEnetCpbBoard_dp83867PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83867PhyCfg),
        },
        .sgmiiMode = ENET_MAC_SGMIIMODE_INVALID,
        .linkCfg   = { ENET_SPEED_AUTO, ENET_DUPLEX_AUTO },
    },
    {   /* "CPSW3G" */
        .macPort   = ENET_MAC_PORT_2,
        .mii       = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg    =
        {
            .phyAddr         = 1U,
            .isStrapped      = BFALSE,
            .skipExtendedCfg = BFALSE,
            .extendedCfg     = &gEnetCpbBoard_dp83867PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83867PhyCfg),
        },
        .sgmiiMode = ENET_MAC_SGMIIMODE_INVALID,
        .linkCfg   = { ENET_SPEED_AUTO, ENET_DUPLEX_AUTO },
    },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwBoard_init(uint32_t flags)
{
    int32_t boardStatus = ENET_SOK;

    EnetAppUtils_enableClocks(ENET_CPSW_3G, 0U);

    return boardStatus;
}

int32_t EthFwBoard_validateMacPorts(Enet_MacPort* enabledMacPortList,
                                    uint8_t numEnabledMacPortList,
                                    Enet_MacPort* gptpEnabledPortList,
                                    uint8_t numGptpEnabledPortList
                                    )
{
    int32_t status = ENET_SOK;
    return status;
}

uint32_t EthFwBoard_getMacPorts(Enet_MacPort macPorts[ENET_MAC_PORT_NUM])
{
    uint32_t num = 0U;
    uint32_t req;
    uint32_t i;

    memset(macPorts, 0, sizeof(*macPorts));

    req = EnetUtils_min(ENET_MAC_PORT_NUM, ENET_ARRAYSIZE(gEthFw_cpbMacPortCfg));
    for (i = 0U; i < req; i++)
    {
        macPorts[num++] = gEthFw_cpbMacPortCfg[i].macPort;
    }

    return num;
}

static const EthFwBoard_MacPortCfg *EthFwBoard_findPortCfg(Enet_MacPort macPort)
{
    const EthFwBoard_MacPortCfg *portCfg = NULL;
    uint32_t i;

    for (i = 0U; i < ENET_ARRAYSIZE(gEthFw_cpbMacPortCfg); i++)
    {
        if (gEthFw_cpbMacPortCfg[i].macPort == macPort)
        {
            portCfg = &gEthFw_cpbMacPortCfg[i];
            break;
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

}

static void EthFwBoard_configCpswClocks(void)
{
    /* Enable CPPI_CLK_CLK and set RGMII_MHZ_[5,50,250]_CLK clock rate */
    EnetAppUtils_enableClocks(ENET_CPSW_3G, 0U);
}

uint32_t EthFwBoard_getMacAddrPool(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                   uint32_t poolSize)
{
    uint32_t allocCnt = 0U;
    uint32_t staticCnt = 0U;

    allocCnt = EthFwBoard_getMacAddrPoolEeprom(macAddr, poolSize);

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
    int32_t boardStatus;
    uint32_t allocCnt = 0U;

    /* Read number of MAC addresses in EFuse and EEPROM */
    boardStatus = EnetAppSoc_fillMacAddrList(macAddr, poolSize, &allocCnt);
    EnetAppUtils_assert(boardStatus == ENET_SOK);

    if (allocCnt == 0U)
    {
        ETHFWTRACE_ERR(ETHFW_EALLOC, "No MAC addresses read from GESI and/or QENET boards");
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

    macAddrCnt = ENET_ARRAYSIZE(macAddrBuf);

    /* Save only those required to meet the max number of MAC entries */
    allocCnt = EnetUtils_min(macAddrCnt, poolSize);
    memcpy(&macAddr[0U][0U], &macAddrBuf[0U][0U], allocCnt * ENET_MAC_ADDR_LEN);

    return allocCnt;
}

