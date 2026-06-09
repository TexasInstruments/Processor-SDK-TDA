/*
 *  Copyright (c) Texas Instruments Incorporated 2024-2025
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
 * \ingroup DRV_ENETPHY
 * \defgroup PHY_COMMON_H TI PHY COMMON
 *
 * TI PHY COMMON for Ethernet PHY.
 *
 * @{
 */
#ifndef PHY_COMMON_H_
#define PHY_COMMON_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

#define ETHPHYDRV_MAX_OBJ_SIZE            (64) /* to meet the size of Phy_Obj_t */

/*! \brief Macro to perform round-up division. */
#define PHY_DIV_ROUNDUP(val, div)         (((val) + (div) - 1) / (div))

/*! \brief Macro to set bit at given bit position. */
#define PHY_BIT(n)                        (1U << (n))

/*! \brief Build-time config option is enabled. */
#define PHY_ON                                     (1U)
/*! \brief Build-time config option is disabled. */
#define PHY_OFF                                    (0U)
/*! \brief Preprocessor check if config option is enabled. */
#define PHY_CFG_IS_ON(name)                        ((PHY_CFG_ ## name) == PHY_ON)
/*! \brief Preprocessor check if config option is disabled. */
#define PHY_CFG_IS_OFF(name)                       ((PHY_CFG_ ## name) == PHY_OFF)

/*!
 * \anchor Phy_ErrorCodes
 * \name   Ethernet PHY driver error codes
 *
 * Error codes returned by the Ethernet PHY driver APIs.
 *
 * @{
 */

/*! \brief Success. */
#define PHY_SOK                           ( (int32_t) (0))
/*! \brief Generic failure error condition (typically caused by hardware). */
#define PHY_EFAIL                         (-(int32_t) (1))
/*! \brief Bad arguments (i.e. NULL pointer). */
#define PHY_EBADARGS                      (-(int32_t) (2))  (-(int32_t) (2))
/*! \brief Invalid parameters (i.e. value out-of-range). */
#define PHY_EINVALIDPARAMS                (-(int32_t) (3))
/*! \brief Time out while waiting for a given condition to happen. */
#define PHY_ETIMEOUT                      (-(int32_t) (4))
/*! \brief Allocation failure. */
#define PHY_EALLOC                        (-(int32_t) (8))
/*! \brief Operation not permitted. */
#define PHY_EPERM                         (PHY_EALLOC - 4)
/*! \brief Operation not supported. */
#define PHY_ENOTSUPPORTED                 (PHY_EALLOC - 5)

/*! @} */

/* PHY Register Definitions */

/*! \brief Basic Mode Control Register (BMCR) */
#define PHY_BMCR                              (0x00U)
/*! \brief Basic Mode Status Register (BMSR) */
#define PHY_BMSR                              (0x01U)
/*! \brief PHY Identifier Register #1 (PHYIDR1) */
#define PHY_PHYIDR1                           (0x02U)
/*! \brief PHY Identifier Register #2 (PHYIDR2) */
#define PHY_PHYIDR2                           (0x03U)
/*! \brief Auto-Negotiation Advertisement Register (ANAR) */
#define PHY_ANAR                              (0x04U)
/*! \brief Auto-Negotiation Link Partner Abilitiy Register (ANLPAR) */
#define PHY_ANLPAR                            (0x05U)
/*! \brief Auto-Negotiation Expansion Register (ANER) */
#define PHY_ANER                              (0x06U)
/*! \brief Auto-Negotiation NP TX Register (ANNPTR) */
#define PHY_ANNPTR                            (0x07U)
/*! \brief Auto-Neg NP RX Register (ANNPRR) */
#define PHY_ANNPRR                            (0x08U)
/*! \brief 1000BASE-T Control Register (GIGCR) */
#define PHY_GIGCR                             (0x09U)
/*! \brief 1000BASE-T Status Register (GIGSR) */
#define PHY_GIGSR                             (0x0AU)
/*! \brief MMD Access Control Register */
#define PHY_MMD_CR                            (0x0DU)
/*! \brief MMD Access Data Register */
#define PHY_MMD_DR                            (0x0EU)
/*! \brief 1000BASE-T Extended Status Register (GIGESR) */
#define PHY_GIGESR                            (0x0FU)

/* MMD_CR register definitions */
#define MMD_CR_ADDR                           (0x0000U)
#define MMD_CR_DATA_NOPOSTINC                 (0x4000U)
#define MMD_CR_DATA_POSTINC_RW                (0x8000U)
#define MMD_CR_DATA_POSTINC_W                 (0xC000U)
#define MMD_CR_DEVADDR                        (0x001FU)

/* BMCR register definitions */
#define PHY_BMCR_RESET                            PHY_BIT(15)
#define PHY_BMCR_LOOPBACK                         PHY_BIT(14)
#define PHY_BMCR_SPEED100                         PHY_BIT(13)
#define PHY_BMCR_ANEN                             PHY_BIT(12)
#define PHY_BMCR_PWRDOWN                          PHY_BIT(11)
#define PHY_BMCR_ISOLATE                          PHY_BIT(10)
#define PHY_BMCR_ANRESTART                        PHY_BIT(9)
#define PHY_BMCR_FD                               PHY_BIT(8)
#define PHY_BMCR_SPEED1000                        PHY_BIT(6)

/*! \brief Max extended configuration size, arbitrarily chosen. */
#define PHY_EXTENDED_CFG_SIZE_MAX         (128U)


/*! \brief 10-Mbps, half-duplex capability mask. */
#define PHY_LINK_CAP_HD10                 PHY_BIT(1)
/*! \brief 10-Mbps, full-duplex capability mask. */
#define PHY_LINK_CAP_FD10                 PHY_BIT(2)
/*! \brief 100-Mbps, half-duplex capability mask. */
#define PHY_LINK_CAP_HD100                PHY_BIT(3)
/*! \brief 100-Mbps, full-duplex capability mask. */
#define PHY_LINK_CAP_FD100                PHY_BIT(4)
/*! \brief 1-Gbps, half-duplex capability mask. */
#define PHY_LINK_CAP_HD1000               PHY_BIT(5)
/*! \brief 1-Gbps, full-duplex capability mask. */
#define PHY_LINK_CAP_FD1000               PHY_BIT(6)
/*! \brief 10-Mbps, full and half-duplex capability mask. */
#define PHY_LINK_CAP_10                   (PHY_LINK_CAP_HD10 | PHY_LINK_CAP_FD10)
/*! \brief 100-Mbps, full and half-duplex capability mask. */
#define PHY_LINK_CAP_100                  (PHY_LINK_CAP_HD100 | PHY_LINK_CAP_FD100)
/*! \brief 1-Gbps, full and half-duplex capability mask. */
#define PHY_LINK_CAP_1000                 (PHY_LINK_CAP_HD1000 | PHY_LINK_CAP_FD1000)
/*! \brief Auto-negotiation mask with all duplexity and speed values set. */
#define PHY_LINK_CAP_ALL                  (PHY_LINK_CAP_HD10 | PHY_LINK_CAP_FD10 |   \
                                           PHY_LINK_CAP_HD100 | PHY_LINK_CAP_FD100 |  \
                                           PHY_LINK_CAP_HD1000 | PHY_LINK_CAP_FD1000)

/*! \brief 10-Mbps, half-duplex advertisement flag. */
#define PHY_LINK_ADV_HD10                 PHY_BIT(1)
/*! \brief 10-Mbps, full-duplex advertisement flag. */
#define PHY_LINK_ADV_FD10                 PHY_BIT(2)
/*! \brief 100-Mbps, half-duplex advertisement flag. */
#define PHY_LINK_ADV_HD100                PHY_BIT(3)
/*! \brief 100-Mbps, full-duplex advertisement flag. */
#define PHY_LINK_ADV_FD100                PHY_BIT(4)
/*! \brief 1-Gbps, half-duplex advertisement flag. */
#define PHY_LINK_ADV_HD1000               PHY_BIT(5)
/*! \brief 1-Gbps, full-duplex advertisement flag. */
#define PHY_LINK_ADV_FD1000               PHY_BIT(6)

/*! \brief  Fast link down option - energy lost */
#define PHY_FAST_LINK_DOWN_ENERGY_LOST              PHY_BIT(0)
/*! \brief  Fast link down option - MSE error */
#define PHY_FAST_LINK_DOWN_MSE                      PHY_BIT(1)
/*! \brief  Fast link down option - MLT3 errors */
#define PHY_FAST_LINK_DOWN_MLT3_ERRORS              PHY_BIT(2)
/*! \brief  Fast link down option - RX error */
#define PHY_FAST_LINK_DOWN_RX_ERR                   PHY_BIT(3)
/*! \brief  Fast link down option - decrambler sync lost */
#define PHY_FAST_LINK_DOWN_DESCRAMBLER_SYNC_LOSS    PHY_BIT(4)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */


typedef struct Phy_Version_s
{
    /*! Organizationally Unique Identifier (OUI) */
    uint32_t oui;
    /*! Manufacturer's model number */
    uint32_t model;
    /*! Revision number */
    uint32_t revision;
} Phy_Version;

typedef enum Phy_Mii_e
{
    /*! \brief MII interface */
    PHY_MAC_MII_MII = 0U,

    /*! \brief RMII interface */
    PHY_MAC_MII_RMII,

    /*! \brief GMII interface */
    PHY_MAC_MII_GMII,

    /*! \brief RGMII interface */
    PHY_MAC_MII_RGMII,

    /*! \brief SGMII interface */
    PHY_MAC_MII_SGMII,

    /*! \brief QSGMII interface */
    PHY_MAC_MII_QSGMII,
} Phy_Mii;

typedef enum Phy_Link_SpeedDuplex_e
{
    /*! \brief Auto-Negotiation */
    PHY_LINK_AUTONEG = 0U,

    /*! \brief full-duplex, 10Mbps */
    PHY_LINK_FD10,

    /*! \brief full-duplex, 100Mbps */
    PHY_LINK_FD100,

    /*! \brief full-duplex, 1000Mbps */
    PHY_LINK_FD1000,

    /*! \brief half-duplex, 10Mbps */
    PHY_LINK_HD10,

    /*! \brief half-duplex, 100Mbps */
    PHY_LINK_HD100,

    /*! \brief half-duplex, 1000Mbps */
    PHY_LINK_HD1000,

    /*! \brief Invalid Speed Duplex */
    PHY_LINK_INVALID,
} Phy_Link_SpeedDuplex;

typedef enum Phy_AutoNegCtrl_e
{
    /*! \brief Disable Auto Negotiation */
    PHY_AUTO_NEGOTIATION_CTRL_DISABLE = 0U,

    /*! \brief Enable Auto Negotiation */
    PHY_AUTO_NEGOTIATION_CTRL_ENABLE,

    /*! \brief Restart Auto Negotiation*/
    PHY_AUTO_NEGOTIATION_CTRL_RESTART,

    /*! \brief Enable and Restart Auto Negotiation */
    PHY_AUTO_NEGOTIATION_CTRL_ENABLE_AND_RESTART,
}Phy_AutoNegCtrl;

typedef struct
{
    int32_t (*EnetPhy_readReg)(void* pArgs, uint32_t reg, uint16_t *val);

    int32_t (*EnetPhy_writeReg)(void*  pArgs, uint32_t reg, uint16_t val);

    int32_t (*EnetPhy_rmwReg)(void*  pArgs, uint32_t reg, uint16_t mask,
            uint16_t val);

    int32_t (*EnetPhy_readExtReg)(void*  pArgs, uint32_t reg,
            uint16_t *val);

    int32_t (*EnetPhy_writeExtReg)(void*  pArgs, uint32_t reg,
            uint16_t val);

    /*!Argument that needs to be passed to the above callbacks */
    void* pArgs;

} Phy_RegAccessCb_t;

typedef uint8_t EthPhyDrv_Handle[ETHPHYDRV_MAX_OBJ_SIZE];

typedef struct
{

    struct
    {

        /*!
         * \brief Driver name.
         *
         * Name of the PHY-specific driver.
         */
        const char *name;

        /*!
         * \brief Check if driver supports a PHY model identified by its version.
         *
         * PHY-specific function that drivers must implement for upper check if the
         * PHY driver supports a PHY model identified by its version from ID1 and
         * ID2 registers.
         *
         * Note that a given PHY driver can support multiple PHY models.
         *
         * \param hPhy     PHY device handle
         * \param version  PHY version from ID registers
         *
         * \return Whether PHY model is supported or not
         */

        bool (*isPhyDevSupported)(EthPhyDrv_Handle hPhy,
                                    const void *pVersion);

        /*!
         * \brief Check if driver supports a MII interface type.
         *
         * PHY-specific function that drivers must implement for upper layer to check
         * whether a MAC mode is supported by the PHY driver or not.
         *
         * \param hPhy     PHY device handle
         * \param mii      MII interface
         *
         * \return Whether MII interface type is supported or not
         */
        bool (*isMacModeSupported)(EthPhyDrv_Handle hPhy,
                                    Phy_Mii mii);

        /*!
         * \brief PHY bind.
         *
         * PHY-specific function that binds the driver handle and register
         * access functions to specific PHY device.
         *
         * \param hPhy              PHY device handle
         * \param phyAddr           PHY address
         * \param pRegAccessCb      PHY register access function pointers
         */

        void (*bind)(EthPhyDrv_Handle* hPhy,
                        uint8_t phyAddr,
                        Phy_RegAccessCb_t* pRegAccessCb);

        /*!
         * \brief PHY specific configuration.
         *
         * PHY-specific function that drivers must implement to configure the PHY
         * device.  The configuration can be composed of generic and PHY-specific
         * parameter (via extended config).
         *
         * \param hPhy     PHY device handle
         * \param cfg      PHY configuration parameter
         * \param mii      MII interface
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*config)(EthPhyDrv_Handle hPhy,
                            const void *pExtCfg,
                            const uint32_t extCfgSize,
                            Phy_Mii mii,
                            bool loopbackEn);

        /*!
         * \brief PHY specific soft reset.
         *
         * PHY-specific function that drivers must implement to start a soft-reset
         * operation.
         *
         * \param hPhy     PHY device handle
         */
        void (*reset)(EthPhyDrv_Handle hPhy);

        /*!
         * \brief PHY specific soft reset status.
         *
         * PHY-specific function that drivers must implement to check if soft-reset
         * operation is complete.
         *
         * \param hPhy     PHY device handle
         *
         * \return Whether soft-reset is complete or not.
         */
        bool (*isResetComplete)(EthPhyDrv_Handle hPhy);

        /*!
         * \brief Read PHY register.
         *
         * PHY-specific function that drivers must implement to read register.
         *
         * \param hPhy     PHY device handle
         * \param reg      Register number
         * \param val      Pointer to the read value
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*readReg) (EthPhyDrv_Handle hPhy,
                            uint32_t reg,
                            uint16_t *val);

        /*!
         * \brief Write PHY register.
         *
         * PHY-specific function that drivers must implement to write extended
         * registers.
         *
         * \param hPhy     PHY device handle
         * \param reg      Register number
         * \param val      Value to be written
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*writeReg) (EthPhyDrv_Handle hPhy,
                             uint32_t reg,
                             uint16_t val);

        /*!
         * \brief Read PHY extended register.
         *
         * PHY-specific function that drivers must implement to read extended
         * registers.
         *
         * \param hPhy     PHY device handle
         * \param reg      Register number
         * \param val      Pointer to the read value
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*readExtReg)(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t* val);

        /*!
         * \brief Write PHY extended register.
         *
         * PHY-specific function that drivers must implement to write extended
         * registers.
         *
         * \param hPhy     PHY device handle
         * \param reg      Register number
         * \param val      Value to be written
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*writeExtReg)(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t val);

        /*!
         * \brief Read-modify-write PHY extended register.
         *
         * PHY-specific function that drivers must implement to read-write-modify
         * extended registers.
         *
         * \param group      User group (use 0 if single group is supported)
         * \param phyAddr    PHY device address
         * \param reg        Register address
         * \param val        Value read from register
         */
        int32_t (*rmwExtReg)(EthPhyDrv_Handle hPhy,
                            uint32_t reg,
                            uint16_t mask,
                            uint16_t* val);

        /*! Print PHY registers */
        void (*printRegs)(EthPhyDrv_Handle hPhy);

        /*!
         * \brief Adjust PHY PTP clock frequency.
         *
         * Optional PHY function to adjust PTP clock frequency.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param ppb      Part per billion
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*adjPtpFreq)(EthPhyDrv_Handle hPhy,
                              int64_t ppb);

        /*!
         * \brief Adjust PHY PTP clock phase.
         *
         * Optional PHY function to adjust PTP clock phase.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param offset   Offset to current clock time in nanosec unit.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*adjPtpPhase)(EthPhyDrv_Handle hPhy,
                              int64_t offset);

        /*!
         * \brief Get current PHY PTP clock time.
         *
         * Optional PHY function to get current PHY PTP clock time.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param ts64     Output current PTP clock time in nanosec unit.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getPtpTime)(EthPhyDrv_Handle hPhy,
                              uint64_t *ts64);

        /*!
         * \brief Set PHY PTP clock time.
         *
         * Optional PHY function to set PHY PTP clock time.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param ts64     PTP time in nanosec unit will be set.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*setPtpTime)(EthPhyDrv_Handle hPhy,
                              uint64_t ts64);

        /*!
         * \brief Get PHY PTP TX packet timestamp.
         *
         * Optional PHY function to get PHY PTP TX packet timestamp.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param domain   PTP domain (in the packet header)
         * \param msgType  PTP message type (in the packet header)
         * \param seqId    PTP packet sequence ID (in the packet header)
         * \param ts64     Output PTP TX packet timestamp in nanosec unit.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getPtpTxTime)(EthPhyDrv_Handle hPhy,
                                uint32_t domain,
                                uint32_t msgType,
                                uint32_t seqId,
                                uint64_t *ts64);

        /*!
         * \brief Get PHY PTP RX packet timestamp.
         *
         * Optional PHY function to get PHY PTP RX packet timestamp.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param domain   PTP domain (in the packet header)
         * \param msgType  PTP message type (in the packet header)
         * \param seqId    PTP packet sequence ID (in the packet header)
         * \param ts64     Output PTP RX packet timestamp in nanosec unit.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getPtpRxTime)(EthPhyDrv_Handle hPhy,
                                uint32_t domain,
                                uint32_t msgType,
                                uint32_t seqId,
                                uint64_t *ts64);

        /*!
         * \brief Add PHY PTP TX packet info to a waiting TX timestamp list.
         *
         * Optional PHY function to get PHY PTP TX packet timestamp.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param domain   PTP domain (in the packet header)
         * \param msgType  PTP message type (in the packet header)
         * \param seqId    PTP packet sequence ID (in the packet header)
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*waitPtpTxTime)(EthPhyDrv_Handle hPhy,
                                 uint32_t domain,
                                 uint32_t msgType,
                                 uint32_t seqId);

        /*!
         * \brief Process PHY status frame.
         *
         * Optional PHY function to process PHY status frame.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param frame    Ethernet PHY status frame
         * \param size     Frame size
         * \param types    Types of processed frame
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*procStatusFrame)(EthPhyDrv_Handle hPhy,
                                   uint8_t *frame,
                                   uint32_t size,
                                   uint32_t *types);

        /*!
         * \brief Get PHY status frame header.
         *
         * Optional PHY function to get the Ethernet header of the PHY status frame.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param ethhdr   Buffer to get the ethernet header of the PHY status frame.
         * \param size     Buffer size (at least 14 bytes)
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getStatusFrameEthHeader)(EthPhyDrv_Handle hPhy,
                                           uint8_t *ethhdr,
                                           uint32_t size);

        /*!
         * \brief Enable or disable the PHY PTP module
         *
         * Optional PHY function to enable or disable the PHY PTP module.
         * This function can only be supported when the PHY has a built-in PTP clock.
         *
         * \param hPhy     PHY device handle
         * \param on       Flag indicate enable (on=true) or disable(on=false) PTP module
         * \param srcMacStatusFrameType      The PHY-specific src MAC of the status frame.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*enablePtp)(EthPhyDrv_Handle hPhy,
                             bool on,
                             uint32_t srcMacStatusFrameType);

        /*!
         * \brief Provide timer tick to the driver.
         *
         * Provide timer tick to the driver.
         *
         * \param hPhy     PHY device handle
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*tickDriver)(EthPhyDrv_Handle hPhy);

        /*!
         * \brief Enable/Disable an event capture on a PHY GPIO pin.
         *
         * Optional PHY function to enable/disable an event capture on a PHY GPIO pin.
         * This function can only be supported when the PHY has a built-in PTP clock
         * that support event capture.
         *
         * \param hPhy     PHY device handle
         * \param eventIdx Event index
         * \param falling  Capture event on falling edge or rising edge if falling is false.
         * \param on       Enable when on is true, otherwise disable the event.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*enableEventCapture)(EthPhyDrv_Handle hPhy, uint32_t eventIdx,
                    bool falling, bool on);

        /*!
         * \brief Enable/Disable trigger output on a GPIO pin.
         *
         * Optional PHY function to enable/disable trigger output on a GPIO pin.
         * This function can only be supported when the PHY has a built-in PTP clock
         * that support trigger output.
         *
         * \param hPhy       PHY device handle
         * \param triggerIdx Trigger index
         * \param start      Start trigger time in nanosec unit.
         * \param period     Period of the pulse in nanosec unit.
         *                   Disable the trigger if the period is equal to 0.
         * \param repeat     Repeated pulse or one shot pulse if repeat is false.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*enableTriggerOutput)(EthPhyDrv_Handle hPhy, uint32_t triggerIdx,
                    uint64_t start, uint64_t period, bool repeat);

        /*!
         * \brief Get event timestamp
         *
         * Get event timestamp
         *
         * \param hPhy       PHY device handle
         * \param eventIdx   Output event index
         * \param seqId      Output event sequence identifier
         * \param ts64       Output event timestamp
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getEventTs)(EthPhyDrv_Handle hPhy, uint32_t *eventIdx,
                    uint32_t *seqId, uint64_t *ts64);
        /*!
         * \brief config Media Clock
         *
         * config Media Clock
         *
         * \param hPhy       PHY device handle
         * \param isMaster   Media Clock Master/Slave Mode
         * \param enTrigOut  If true, starts 100Hz phase aligned signal
         *                   to the media clock at LED0
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*configMediaClock)(EthPhyDrv_Handle hPhy, bool isMaster,
            uint8_t *streamIDMatchValue, bool enTrigOut);
        /*!
         * \brief Nudge Codec Clock
         *
         * Nudge Codec Clock
         *
         * \param hPhy         PHY device handle
         * \param NudgeValue   int8_t value to nudge Codec clock.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*nudgeCodecClock)(EthPhyDrv_Handle hPhy, int8_t nudgeValue);

        /*!
         * \brief PHY specific software restart.
         *
         * PHY-specific function that drivers must implement to start a soft-restart
         * operation.
         *
         * \param hPhy     PHY device handle
         *
         */
        void (*restart)(EthPhyDrv_Handle hPhy);

        /*!
         * \brief PHY specific soft reset status.
         *
         * PHY-specific function that drivers must implement to check if soft-restart
         * operation is complete.
         *
         * \param hPhy       PHY device handle
         * \param pCompleted Pointer to place holder where the result will be stored
         *                   false - software restart ongoing
         *                   true  - software restart completed
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isRestartComplete)(EthPhyDrv_Handle hPhy, bool* pCompleted);

        /*!
         * \brief Provides PHY ID value as 32-bit value
         *
         * The 32-bit value combines content of IEEE PHY Identifier Register 1 (Register 2)
         * to higher 16-bit and content of IEEE PHY Identifier Register 2 (Register 3)
         * to lower 16-bit.
         *
         * \param hPhy       PHY device handle
         * \param pId        PHY ID value
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getId)(EthPhyDrv_Handle hPhy, uint32_t *pId);

        /*!
         * \brief Controls IEEE power down feature
         *
         * Access IEEE Control Register (Register 0) to control power down feature
         * by setting/clearing "Power Down" bit (Bit 11).
         *
         * \param hPhy       PHY device handle
         * \param control    When set the IEEE power down feature will be activated.
         *                   Normal mode otherwise.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*ctrlPowerDown)(EthPhyDrv_Handle hPhy, bool control);

        /*!
         * \brief Checks if IEEE power down feature is enabled
         *
         * Access IEEE Control Register (Register 0) to check power down feature
         * by reading "Power Down" bit (Bit 11).
         *
         * \param hPhy        PHY device handle
         * \param pActive     Pointer to place holder where the result will be stored
         *                    false - normal mode
         *                    true  - IEEE power down mode
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isPowerDownActive)(EthPhyDrv_Handle hPhy, bool *pActive);

        /*!
         * \brief Based on flags will enable required advertisement for Auto Negotiation phase.
         *
         * Access IEEE Auto-Negotiation Advertisement Register (Register 4) and IEEE MASTER-SLAVE
         * Control Register (Register 9) to set specified advertisement bits. 10HD (Bit 5), 10FD
         * (Bit 6), 100HD (Bit 7) and 100FD (Bit 8) on Auto-Negotiation Advertisement Register.
         * The 1000HD (Bit 10) and 1000FD (Bit 11) on MASTER-SLAVE Control Register.
         *
         * \param hPhy           PHY device handle
         * \param advertisement  Contains advertisement flags to enable as described below.
         *                       PHY_LINK_ADV_HD10   - 10Mbps, Half-Duplex advertisement
         *                       PHY_LINK_ADV_FD10   - 10Mbps, Full-Duplex advertisement
         *                       PHY_LINK_ADV_HD100  - 100Mbps, Half-Duplex advertisement
         *                       PHY_LINK_ADV_FD100  - 100Mbps, Full-Duplex advertisement
         *                       PHY_LINK_ADV_HD1000 - 1000Mbps, Half-Duplex advertisement
         *                       PHY_LINK_ADV_FD1000 - 1000Mbps, Full-Duplex advertisement
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*enableAdvertisement)(EthPhyDrv_Handle hPhy, uint32_t advertisement);

        /*!
         * \brief Based on flags will disable required advertisement for Auto Negotiation phase
         *
         * Access IEEE Auto-Negotiation Advertisement Register (Register 4) and IEEE MASTER-SLAVE
         * Control Register (Register 9) to clear specified advertisement bits. 10HD (Bit 5), 10FD
         * (Bit 6), 100HD (Bit 7) and 100FD (Bit 8) on Auto-Negotiation Advertisement Register.
         * The 1000HD (Bit 10) and 1000FD (Bit 11) on MASTER-SLAVE Control Register.
         *
         * \param hPhy           PHY device handle
         * \param advertisement  Contains advertisement flags to disable as described below.
         *                       PHY_LINK_ADV_HD10   - 10Mbps, Half-Duplex advertisement
         *                       PHY_LINK_ADV_FD10   - 10Mbps, Full-Duplex advertisement
         *                       PHY_LINK_ADV_HD100  - 100Mbps, Half-Duplex advertisement
         *                       PHY_LINK_ADV_FD100  - 100Mbps, Full-Duplex advertisement
         *                       PHY_LINK_ADV_HD1000 - 1000Mbps, Half-Duplex advertisement
         *                       PHY_LINK_ADV_FD1000 - 1000Mbps, Full-Duplex advertisement
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*disableAdvertisement)(EthPhyDrv_Handle hPhy, uint32_t advertisement);

        /*!
         * \brief Controls Auto Negotiation feature
         *
         * Access IEEE Control Register (Register 0) to control Auto-Negotiation feature
         * by setting/clearing "Restart Auto-Negotiation" bit (Bit 9) and "Auto-Negotiation
         * Enable" bit (Bit 12).
         *
         * Advertisement or fixed speed/duplex mode settings need to be applied before.
         *
         * \param hPhy       PHY device handle.
         * \param control    Following control combinations are possible \ref Phy_AutoNegCtrl.
         *
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*ctrlAutoNegotiation)(EthPhyDrv_Handle hPhy, uint32_t control);

        /*!
         * \brief Provides Link-Partner Auto-Negotiation Able flag
         *
         * Access IEEE Auto-Negotiation Expansion Register (Register 6) and
         * reads "Link-Partner is Auto-Negotiation Able" bit (Bit 0).
         *
         * \param hPhy  PHY device handle.
         * \param pAble Pointer to place holder where the result will be stored.
         *              false - Link-Partner is not able to participate on Auto-Negotiation process
         *              true - Link-Partner is able to participate on Auto-Negotiation process
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isLinkPartnerAutoNegotiationAble)(EthPhyDrv_Handle hPhy, bool *pAble);

        /*!
         * \brief Checks if Auto Negotiation is enabled
         *
         * Access IEEE Control Register (Register 0) to read "Auto-Negotiation Enable" bit (Bit 12).
         *
         * \param hPhy       PHY device handle
         * \param pEnabled   Pointer to place holder where the result will be stored
         *                   false - Auto Negotiation is off
         *                   true - Auto Negotiation is on
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isAutoNegotiationEnabled)(EthPhyDrv_Handle hPhy, bool *pEnabled);

        /*!
         * \brief Checks status of Auto Negotiation process
         *
         * Access IEEE Status Register (Register 1) to read "Auto-Negotiation Complete" bit (Bit 5).
         *
         * \param hPhy        PHY device handle
         * \param pCompleted  Pointer to place holder where the result will be stored
         *                    false - Auto Negotiation process not completed
         *                    true - Auto Negotiation process completed
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isAutoNegotiationComplete)(EthPhyDrv_Handle hPhy, bool *pCompleted);

        /*!
         * \brief Checks status of Auto Negotiation restart process
         *
         * Access IEEE Control Register (Register 0) to read "Restart Auto-Negotiation" bit (Bit 9).
         *
         * \param hPhy        PHY device handle
         * \param pCompleted   Pointer to place holder where the result will be stored
         *                    false - Auto Negotiation restart process not completed
         *                    true - Auto Negotiation restart process completed
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isAutoNegotiationRestartComplete)(EthPhyDrv_Handle hPhy, bool *pCompleted);

        /*!
         * \brief Checks link state
         *
         * Access IEEE Status Register (Register 1) to read "Link Status" bit (Bit 2).
         *
         * \param hPhy       PHY device handle
         * \param pLinkUp    Pointer to place holder where the result will be stored
         *                   false - Link down state
         *                   true  - Link up state
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*isLinkUp)(EthPhyDrv_Handle hPhy, bool *pLinkUp);

        /*!
         * \brief Sets speed and duplex settings to control data rate
         *        of the Ethernet link when Auto Negotiation is disabled
         *
         * Access IEEE Control Register (Register 0) to configure speed by setting/clearing
         * of "Speed Selection (MSB)" and "Speed Selection (LSB)" bits (Bit 6 and 13)
         * together with duplex by setting/clearing of "Duplex Mode" bit (Bit 8).
         *
         * \param hPhy           PHY device handle
         * \param settings       Contains speed/duplex combination flag to select.
         *                       \ref Phy_Link_SpeedDuplex
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*setSpeedDuplex)(EthPhyDrv_Handle hPhy, uint32_t settings);

        /*!
         * \brief Provides selected speed and duplex when link is up
         *
         * Accessing vendor specific or extended registers based on type of PHY to provide
         * agreed speed and duplex mode for actual connection.
         *
         * \param hPhy           PHY device handle
         * \param pConfig        Contains speed/duplex combination used for actual
         *                       connection. See \ref Phy_Link_SpeedDuplex.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*getSpeedDuplex)(EthPhyDrv_Handle hPhy, Phy_Link_SpeedDuplex* pConfig);

        /*!
         * \brief Configures MII mode
         *
         * Accessing vendor specific or extended registers based on type of PHY to configure MII mode.
         *
         * \param hPhy           PHY device handle
         * \param mii            MII mode to configure. See \ref Phy_Mii.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        void (*setMiiMode)(EthPhyDrv_Handle hPhy, Phy_Mii mii);

        /*!
         * \brief Enables to declare Full Duplex also in parallel detect link.
         *
         * Accessing vendor specific or extended registers based on type of PHY.
         *
         * \param hPhy       PHY device handle.
         * \param control    Enable/Disable Full-Duplex also in parallel detect link.
         *                   false - disable Full-Duplex in parallel detect link
         *                   true  - enable Full-Duplex in parallel detect link
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*ctrlExtFD)(EthPhyDrv_Handle hPhy, bool control);

        /*!
         * \brief Controls detection of transmit error in odd-nibble boundary.
         *
         * Accessing vendor specific or extended registers based on type of PHY.
         * Some PHY's are not providing such a feature.
         *
         * \param hPhy       PHY device handle.
         * \param control    Enable/Disable Full-Duplex also in parallel detect link.
         *                   false - disable detection of transmit error in odd-nibble boundary.
         *                   true  - enable detection of transmit error in odd-nibble boundary.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*ctrlOddNibbleDetection)(EthPhyDrv_Handle hPhy, bool control);

        /*!
         * \brief Controls detection of Receive Symbol Error during IDLE State.
         *
         * Accessing vendor specific or extended registers based on type of PHY.
         * Some PHY's are not providing such a feature.
         *
         * \param hPhy       PHY device handle.
         * \param control    Enable/Disable Full-Duplex also in parallel detect link.
         *                   false - disable detection of Receive Symbol Error during IDLE State.
         *                   true  - enable detection of Receive Symbol Error during IDLE State.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*ctrlRxErrIdle)(EthPhyDrv_Handle hPhy, bool control);

        /*!
         * \brief Controls fast link down option.
         *
         * Accessing vendor specific or extended registers based on type of PHY.
         * Some PHY's are not providing such a feature.
         *
         * \param hPhy       PHY device handle.
         * \param control    Enable/Disable Full-Duplex also in parallel detect link.
         *                   false - disable detection of Receive Symbol Error during IDLE State.
         *                   true  - enable detection of Receive Symbol Error during IDLE State.
         *
         * \return \ref EnetPhy_ErrorCodes
         */
        int32_t (*ctrlFastLinkDownOption)(EthPhyDrv_Handle hPhy, uint32_t control);
    } fxn;

    EthPhyDrv_Handle hDrv;
} Phy_DrvObj_t;

typedef Phy_DrvObj_t* EthPhyDrv_If;


/* TODO: Move this to private files */
typedef struct
{
    uint8_t phyAddr;
    Phy_RegAccessCb_t regAccessApi;
} Phy_Obj_t;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t GenericPhy_readReg(EthPhyDrv_Handle hPhy,
                           uint32_t reg,
                           uint16_t *val);

int32_t GenericPhy_writeReg(EthPhyDrv_Handle hPhy,
                            uint32_t reg,
                            uint16_t val);

int32_t GenericPhy_readExtReg(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t* val);

int32_t GenericPhy_writeExtReg(EthPhyDrv_Handle hPhy,
                                uint32_t reg,
                                uint16_t val);

void GenericPhy_reset(EthPhyDrv_Handle hPhy);

bool GenericPhy_isResetComplete(EthPhyDrv_Handle hPhy);

int32_t GenericPhy_ctrlPowerDown(EthPhyDrv_Handle hPhy,
                                 bool control);

int32_t GenericPhy_isPowerDownActive(EthPhyDrv_Handle hPhy,
                                     bool *active);

int32_t GenericPhy_enableAdvertisement(EthPhyDrv_Handle hPhy,
                                   uint32_t advertisement);

int32_t GenericPhy_disableAdvertisement(EthPhyDrv_Handle hPhy,
                                       uint32_t advertisement);

int32_t GenericPhy_ctrlAutoNegotiation(EthPhyDrv_Handle hPhy,
                                       uint32_t control);

int32_t GenericPhy_isLinkPartnerAutoNegotiationAble (EthPhyDrv_Handle hPhy,
                                                     bool *pAble);

int32_t GenericPhy_isAutoNegotiationEnabled(EthPhyDrv_Handle hPhy,
                                            bool *enabled);

int32_t GenericPhy_isAutoNegotiationComplete (EthPhyDrv_Handle hPhy,
                                              bool *completed);

int32_t GenericPhy_isAutoNegotiationRestartComplete (EthPhyDrv_Handle hPhy,
                                                     bool *completed);

int32_t GenericPhy_setSpeedDuplex (EthPhyDrv_Handle hPhy,
                                   uint32_t settings);

int32_t GenericPhy_isLinkUp (EthPhyDrv_Handle hPhy,
                             bool *linkUp);

int32_t GenericPhy_getId(EthPhyDrv_Handle hPhy,
                         uint32_t *id);

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

#endif /* PHY_COMMON_H_ */

/*! @} */
