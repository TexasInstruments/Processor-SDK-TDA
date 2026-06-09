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
 * \file  ethfw_board_utils.h
 *
 * \brief This file contains the type definitions and helper macros of the
 *        Ethernet Firmware board utils library.
 */

#ifndef ETHFW_BOARD_UTILS_H_
#define ETHFW_BOARD_UTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <enet.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_MODULE
 * \defgroup ETHFW_BOARD_UTILS Ethernet Board Utils
 *
 * \brief This section contains APIs for board initialization needed by
 *        Ethernet Firmware.
 *
 * Ethernet Firmware board library is small set of functions intended to initialize
 * board related functionality needed for Ethernet Firmware to run, ie. setup GPIOs
 * to take PHY out of reset, configure SerDes, etc.
 *
 * This utils library has been written to support TI EVMs and can be used as
 * reference when porting Ethernet Firmware to a different platform.
 * @{
 */
/*! @} */

/*!
 * \addtogroup ETHFW_BOARD_UTILS
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*!
 * \anchor EthFwBoard_ConfigFlags
 * \name Configuration flags for EthFw board utils
 *
 * The following configuration flags are used to indicate the board initialization
 * that must be done by EthFw board function EthFwBoard_init().
 *
 * @{
 */

/*! \brief Whether Enet board utils should enable GESI expansion board */
#define ETHFW_BOARD_GESI_ENABLE                (ENET_BIT(0))

/*! \brief Whether Enet board utils should enable ENET expansion board (QSGMII board) */
#define ETHFW_BOARD_QENET_ENABLE               (ENET_BIT(1))

/*! \brief Whether Enet board utils can configure SerDes */
#define ETHFW_BOARD_SERDES_CONFIG              (ENET_BIT(2))

/*! \brief Whether Enet board utils can configure UART for logging */
#define ETHFW_BOARD_UART_ALLOWED               (ENET_BIT(3))

/*! \brief Whether Enet board utils can run I2C-related operations, i.e. read EEPROM */
#define ETHFW_BOARD_I2C_ALLOWED                (ENET_BIT(4))

/*! \brief Whether Enet board utils can set GPIOs, i.e. PHY reset */
#define ETHFW_BOARD_GPIO_ALLOWED               (ENET_BIT(5))

/*! Whether Enet board utils should enable ENET expansion bridge, which is used
 *  to connect two EVMs in MAC-to-MAC mode.  Mutually exclusive with
 *  \ref ETHFW_BOARD_QENET_ENABLE. */
#define ETHFW_BOARD_ENET_BRIDGE_ENABLE         (ENET_BIT(6))

/*! @} */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize board-related functionality needed by Ethernet Firmware.
 *
 * Initializes board related functionality for Ethernet Firmware to run.
 * This includes clocking CPSW, configuring expansion boards, etc.
 *
 * \param flags     Flags to indicate the functionality to be enabled.
 *
 * \return 0 in case of success, error code otherwise.
 */
int32_t EthFwBoard_init(uint32_t flags);

/*!
 * \brief Validate whether macport configuration in Ethernet Firmware is correct.
 *
 * In the absence of a GESI board, any attempt to use GESI specific
 * ports will raise an assert for J721E. Also, gPTP enabled ports should be a
 * subset of total enabled ports.
 *
 * \param enabledMacPortList            Array of all macports owned by EthFw.
 * \param numEnabledMacPortList         Size of enabledMacPortList array.
 * \param gptpEnabledPortList           Array of ports where gPTP support is enabled.
 * \param numGptpEnabledPortList        Size of gptpEnabledPortList array.
 *
 * \return 0 in case of success, error code otherwise.
 */

int32_t EthFwBoard_validateMacPorts(Enet_MacPort* enabledMacPortList,
                                    uint8_t numEnabledMacPortList,
                                    Enet_MacPort* gptpEnabledPortList,
                                    uint8_t numGptpEnabledPortList
                                    );

/*!
 * \brief Get the list of enabled MAC ports.
 *
 * Gets the list of MAC ports enabled in this board implementation.
 *
 * \param macPorts  Array to be populated with list of enabled MAC ports.
 *
 * \return Number of enabled MAC ports.
 */
uint32_t EthFwBoard_getMacPorts(Enet_MacPort macPorts[ENET_MAC_PORT_NUM]);

/*!
 * \brief Set the port configuration parameters for the specified MAC port.
 *
 * Populates the MAC port configuration, MII interface type (RMII, RGMII,
 * Q/SGMII), PHY configuration parameters and link configuration type
 * (manual, auto-negotiation) for the passed MAC port number.
 *
 * \param macPort   MAC port to get its configuration parameters for.
 * \param macCfg    MAC config to be populated: SGMII mode.
 * \param mii       MII interface type to populated: RMII, RGMII, Q/SGMII.
 * \param phyCfg    PHY config to be populated: PHY address, strapping, etc.
 * \param linkCfg   Link config to be populated: manual, auto-negotiation.
 *
 * \return 0 in case of success, error code otherwise.
 */
int32_t EthFwBoard_setPortCfg(Enet_MacPort macPort,
                              CpswMacPort_Cfg *macCfg,
                              EnetMacPort_Interface *mii,
                              EnetPhy_Cfg *phyCfg,
                              EnetMacPort_LinkCfg *linkCfg);

/*!
 * \brief Get the MAC address pool of this board.
 *
 * Populates the passed MAC address pool array with board's MAC addresses.
 * The addresses are usually read from EEPROM, but it's implementation choice
 * (i.e. static pool).
 *
 * \param macAddr   Array to be populated with list of enabled MAC ports.
 * \param poolSize  Pool size, the max number of entries to be populated.
 *
 * \return Actual number of MAC addresses added into pool array.
 */
uint32_t EthFwBoard_getMacAddrPool(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                   uint32_t poolSize);

#if defined(SOC_J722S)                                   
int32_t BoardUtils_enableSerDes(uint32_t serdesInstance);
int32_t PMLIBClkRateGet(uint32_t modId,
                        uint32_t clkId,
                        uint64_t *clkRate);

int32_t PMLIBClkRateSet(uint32_t modId,
                        uint32_t clkId,
                        uint64_t clkRate);

void BoardUtils_setDeviceState(uint32_t moduleId,
                               uint32_t requiredState,
                               uint32_t appFlags);

uint32_t Board_serdesCfgSgmii(uint32_t serdesInstance);

#endif

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* ETHFW_BOARD_UTILS_H_ */
