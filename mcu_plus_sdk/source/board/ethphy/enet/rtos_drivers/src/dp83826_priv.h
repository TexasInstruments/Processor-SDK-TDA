/*
 *  Copyright (c) Texas Instruments Incorporated 2020 - 2025
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
 * \file  dp83826_priv.h
 *
 * \brief This file contains private type definitions and helper macros for the
 *        dp83826 Ethernet PHY.
 */

#ifndef dp83826_PRIV_H_
#define dp83826_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include "phy_common_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief PHY Control Register #1 (CR1) */
#define DP83826_CR1                           (0x09U)

/*! \brief PHY Control Register #2 (CR2)*/
#define DP83826_CR2                           (0x0AU)

/*! \brief PHY Control Register #2 (CR3)*/
#define DP83826_CR3                           (0x0BU)

/*! \brief PHY Status Register (PHYSTS) */
#define DP83826_PHYSTS                        (0x10U)

/*! \brief RMII and Status Register (RCSR) */
#define DP83826_RCSR                          (0x17U)

/*! \brief LED Control Register (LEDCR) */
#define DP83826_LEDCR                         (0x18U)

/*! \brief PHY Control Register (PHYCR) */
#define DP83826_PHYCR                         (0x19U)

/*! \brief PHY Reset Control Register (PHYRCR) */
#define DP83826_PHYRCR                        (0x1FU)

/*! \brief Multi-LED Control Register */
#define DP83826_MLEDCR                        (0x25U)

/*! \brief LED0 GPIO Configuration Register */
#define DP83826_LED0_GPIO_CFG                 (0x303U)

/*! \brief LED1 GPIO Configuration Register */
#define DP83826_LED1_GPIO_CFG                 (0x304U)

/*! \brief LED2 GPIO Configuration Register */
#define DP83826_LED2_GPIO_CFG                 (0x305U)

/*! \brief LED3 GPIO Configuration Register */
#define DP83826_LED3_GPIO_CFG                 (0x306U)

/*! \brief LEDs Configuration Register #1 */
#define DP83826_LEDCFG                        (0x460U)

/*! \brief LEDs Configuration Register #2 */
#define DP83826_LEDCFG2                       (0x469U)

/* RCSR register definitions */
#define RCSR_RMII_MODE                        PHY_BIT(5U)

/* CR1 register definitions */
#define CR1_FASTRXDV_DET                      PHY_BIT(1U)
#define CR1_ROBUSTAUTOMDIX                    PHY_BIT(5U)


/* CR2 register definitions */
#define CR2_EXTFD                             PHY_BIT(5U)
#define CR2_RXER                              PHY_BIT(2U)
#define CR2_ODDNIBBLEDET_DISABLE              PHY_BIT(1U)

/* CR3 register definitions */
#define CR3_FLDMODE_OFFSET                    (0U)
#define CR3_FLDMODE_MASK                      (0x000FU)

/* PHYST register definitions */
#define PHYST_SPEEDSEL_MASK                   (0x0002U)
#define PHYST_SPEEDSEL_100_MBPS               (0x0000U)
#define PHYST_SPEEDSEL_10_MBPS                (0x0002U)
#define PHYST_DUPLEXMODEENV_FD                (0x0004U)

/*! \brief PHY STS bits */
#define DP83826_PHYSTS_LINK	                  PHY_BIT(0)

/* PHYCR register definitions */
#define PHYCR_AUTOMDIX_ENABLE                 (0x8000U)
#define PHYCR_FORCEMDIX_MASK                  (0x4000U)
#define PHYCR_FORCEMDIX_MDIX                  (0x4000U)
#define PHYCR_FORCEMDIX_MDI                   (0x0000U)

#define PHYCR_LEDSBYPASS_OFFSET               (7U)
#define PHYCR_LEDSBYPASS_MASK                 (0x0080U)

/* LEDCR register definitions */
#define LEDCR_BLINKRATE_OFFSET               (9U)
#define LEDCR_BLINKRATE_MASK                 (0x0600U)
#define LEDCR_LINKPOLARITY_OFFSET            (7U)
#define LEDCR_LINKPOLARITY_MASK              (0x0080)
#define LEDCR_LINKDRVEN_OFFSET               (4U)
#define LEDCR_LINKDRVEN_MASK                 (0x0010)
#define LEDCR_LINKDRVVAL_OFFSET              (1U)
#define LEDCR_LINKDRVVAL_MASK                (0x0002)

/* PHYRCR register definitions */
#define PHYRCR_SWRESET                        PHY_BIT(15U)
#define PHYRCR_SWRESTART                      PHY_BIT(14U)

/* MLEDCR register definitions */
#define MLEDCR_LED0_CONFIG_OFFSET             (3U)
#define MLEDCR_LED0_CONFIG_MASK               (0x0038U)

#define MLEDCR_LED0_LINK_OK                   (0x0U)
#define MLEDCR_LED0_RXTXACT                   (0x1U)
#define MLEDCR_LED0_TXACT                     (0x2U)
#define MLEDCR_LED0_RXACT                     (0x3U)
#define MLEDCR_LED0_COLLISION                 (0x4U)
#define MLEDCR_LED0_SPEED_100BTX              (0x5U)
#define MLEDCR_LED0_SPEED_10BTX               (0x6U)
#define MLEDCR_LED0_FULL_DUPLEX               (0x7U)
#define MLEDCR_LED0_BLINK_ACT                 (0x8U)
#define MLEDCR_LED0_STRETCH_ACT               (0x9U)
#define MLEDCR_LED0_100BTFD                   (0xAU)
#define MLEDCR_LED0_LPI_MODE                  (0xBU)
#define MLEDCR_LED0_TXRX_ERROR                (0xCU)
#define MLEDCR_LED0_LINK_LOSS                 (0xDU)
#define MLEDCR_LED0_PRBS_ERROR                (0xEU)

#define MLEDCR_LED0_MLED_EN                   PHY_BIT(0U)

/* LED0_GPIO_CFG register definitions */
#define LED0_GPIO_CFG_CTRL_OFFSET             (0U)
#define LED0_GPIO_CFG_CTRL_MASK               (0x0007U)

/* LED1_GPIO_CFG register definitions */
#define LED1_GPIO_CFG_CTRL_OFFSET             (0U)
#define LED1_GPIO_CFG_CTRL_MASK               (0x0007U)

/* LED2_GPIO_CFG register definitions */
#define LED2_GPIO_CFG_CTRL_OFFSET             (0U)
#define LED2_GPIO_CFG_CTRL_MASK               (0x0007U)

/* LED3_GPIO_CFG register definitions */
#define LED3_GPIO_CFG_CTRL_OFFSET             (0U)
#define LED3_GPIO_CFG_CTRL_MASK               (0x0007U)

/* LEDCFG2 register definitions */
#define LEDCFG_LED3_CTRL_OFFSET               (4U)
#define LEDCFG_LED3_CTRL_MASK                 (0x00F0U)
#define LEDCFG_LED2_CTRL_OFFSET               (8U)
#define LEDCFG_LED2_CTRL_MASK                 (0x0F00U)
#define LEDCFG_LED1_CTRL_OFFSET               (12U)
#define LEDCFG_LED1_CTRL_MASK                 (0xF000U)

#define LEDCFG_LED_LINK_OK                    (0x0U)
#define LEDCFG_LED_RXTXACT                    (0x1U)
#define LEDCFG_LED_TXACT                      (0x2U)
#define LEDCFG_LED_RXACT                      (0x3U)
#define LEDCFG_LED_COLLISION                  (0x4U)
#define LEDCFG_LED_SPEED_100BTX               (0x5U)
#define LEDCFG_LED_SPEED_10BTX                (0x6U)
#define LEDCFG_LED_FULL_DUPLEX                (0x7U)
#define LEDCFG_LED_BLINK_ACT                  (0x8U)
#define LEDCFG_LED_STRETCH_ACT                (0x9U)
#define LEDCFG_LED_100BTFD                    (0xAU)
#define LEDCFG_LED_LPI_MODE                   (0xBU)
#define LEDCFG_LED_TXRX_ERROR                 (0xCU)
#define LEDCFG_LED_LINK_LOSS                  (0xDU)
#define LEDCFG_LED_PRBS_ERROR                 (0xEU)

/* LEDCFG2 register definitions */
#define LEDCFG2_LED1DRVEN_OFFSET              (0U)
#define LEDCFG2_LED1DRVEN_MASK                (0x0001U)
#define LEDCFG2_LED1DRVVAL_OFFSET             (1U)
#define LEDCFG2_LED1DRVVAL_MASK               (0x0002U)
#define LEDCFG2_LED1POLARITY_OFFSET           (2U)
#define LEDCFG2_LED1POLARITY_MASK             (0x0004U)
#define LEDCFG2_LED1RESERVED_OFFSET           (3U)
#define LEDCFG2_LED1RESERVED_MASK             (0x0008U)
#define LEDCFG2_LED2DRVEN_OFFSET              (4U)
#define LEDCFG2_LED2DRVEN_MASK                (0x0010U)
#define LEDCFG2_LED2DRVVAL_OFFSET             (5U)
#define LEDCFG2_LED2DRVVAL_MASK               (0x0020U)
#define LEDCFG2_LED2POLARITY_OFFSET           (6U)
#define LEDCFG2_LED2POLARITY_MASK             (0x0040U)
#define LEDCFG2_LED2RESERVED_OFFSET           (7U)
#define LEDCFG2_LED2RESERVED_MASK             (0x0080U)
#define LEDCFG2_LED3DRVEN_OFFSET              (8U)
#define LEDCFG2_LED3DRVEN_MASK                (0x0100U)
#define LEDCFG2_LED3DRVVAL_OFFSET             (9U)
#define LEDCFG2_LED3DRVVAL_MASK               (0x0200U)
#define LEDCFG2_LED3POLARITY_OFFSET           (10U)
#define LEDCFG2_LED3POLARITY_MASK             (0x0400U)
#define LEDCFG2_LED3RESERVED_OFFSET           (11U)
#define LEDCFG2_LED3RESERVED_MASK             (0x0800U)

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

/* None */

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

#endif /* dp83826_PRIV_H_ */
