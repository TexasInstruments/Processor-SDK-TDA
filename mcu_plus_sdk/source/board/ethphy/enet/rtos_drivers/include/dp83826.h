/*
 *  Copyright (c) Texas Instruments Incorporated 2020
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
 * \file  dp83826.h
 *
 * \brief This file contains the type definitions and helper macros for the
 *        DP83826 Ethernet PHY.
 */

/*!
 * \ingroup  DRV_ENETPHY
 * \defgroup ENETPHY_DP83826 TI DP83826 PHY
 *
 * TI DP83826 RMII Ethernet PHY.
 *
 * @{
 */

#ifndef dp83826_H_
#define dp83826_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

#include "phy_common.h"
#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \brief LED modes (sources).
 */
typedef enum Dp83826_LedMode_e
{
    /*! Link established */
    DP83826_LED_LINK_OK          = 0x0U,

    /*! Receive or transmit activity */
    DP83826_LED_RXTXACT          = 0x1U,

    /*! Transmit activity */
    DP83826_LED_TXACT            = 0x2U,

    /*! Receive activity */
    DP83826_LED_RXACT            = 0x3U,

    /*! Collision detected */
    DP83826_LED_COLLISION        = 0x4U,

    /*! Speed, High for 100BASE-TX */
    DP83826_LED_SPEED_100BTX     = 0x5U,

    /*! Speed, High for 10BASE-T */
    DP83826_LED_SPEED_10BTX      = 0x6U,

    /*! Full-Duplex */
    DP83826_LED_FULL_DUPLEX      = 0x7U,

    /*! LINK OK / BLINK on TX/RX Activity */
    DP83826_LED_BLINK_ACT        = 0x8U,

    /*! Active Stretch Signal */
    DP83826_LED_STRETCH_ACT      = 0x9U,

    /*! MII LINK (100BT+FD) */
    DP83826_LED_100BTFD           = 0xAU,

    /*! LPI Mode (Energy Efficient Ethernet) */
    DP83826_LED_LPI_MODE         = 0xBU,

    /*! TX/RX MII Error */
    DP83826_LED_RXTXERR          = 0xCU,

    /*! Link Lost (remains on until register 0x0001 is read) */
    DP83826_LED_LINK_LOSS        = 0xDU,

    /*! Blink for PRBS error (remains ON for single error, remains until counter is cleared) */
    DP83826_LED_PRBS_ERROR       = 0xEU,
} Dp83826_LedMode;

/*!
 * \brief dp83826 PHY configuration parameters.
 */
typedef struct Dp83826_Cfg_s
{
    /* No extended config parameters at the moment */
} Dp83826_Cfg;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize dp83826 PHY specific config params.
 *
 * Initializes the dp83826 PHY specific configuration parameters.
 *
 * \param cfg       dp83826 PHY config structure pointer
 */
void Dp83826_initCfg(Dp83826_Cfg *cfg);

void Dp83826_bind(EthPhyDrv_Handle* hPhy,
                    uint8_t phyAddr,
                    Phy_RegAccessCb_t* pRegAccessCb);

bool Dp83826_isPhyDevSupported(EthPhyDrv_Handle hPhy,
                                const void *pVersion);

bool Dp83826_isMacModeSupported(EthPhyDrv_Handle hPhy,
                                Phy_Mii mii);

int32_t Dp83826_config(EthPhyDrv_Handle hPhy,
                        const void *pExtCfg,
                        const uint32_t extCfgSize,
                        Phy_Mii mii,
                        bool loopbackEn);

void Dp83826_printRegs(EthPhyDrv_Handle hPhy);

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

#endif /* dp83826_H_ */

/*! @} */