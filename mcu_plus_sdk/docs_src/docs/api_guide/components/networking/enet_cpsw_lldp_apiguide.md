# Ethernet LLDP Stack - API and Integration Guide {#ENET_CPSW_LLDP}

[TOC]

# Pre-requisites
Reader are expected to have basic knowledge on below IEEE specifications
- Standard ethernet (IEEE 802.1)
- Link Layer Discovery Protocol - LLDP (IEEE 802.1AB-2016)
- LLDP Amendment 1: YANG Data Model (IEEE 802.1ABcu-2021)

# Introduction
This guide is intended to enhance user's understanding of the LLDP stack and provide guidance on how to seamlessly integrate LLDP modules into their own applications.

# Demo and Examples
\ref EXAMPLES_ENET_CPSW_LLDP

# TSN Stack

## Compilation
The compilation of library is similar with \ref ENET_CPSW_TSN_GPTP.

## Modules

The TSN Stack library is composed of the following source modules:

 Module Name  | lcoation | Description
 -------------|-----------|-----------
 Unibase      | `${SDK_INSTALL_PATH}/source/networking/tsn/tsn_unibase` | Universal utility libraries that are platform-independent
 Combase      | `${SDK_INSTALL_PATH}/source/networking/tsn/tsn_combase` | Communication utility libraries that provide support for functions like sockets, mutexes, and semaphores
 Uniconf      | `${SDK_INSTALL_PATH}/source/networking/tsn/tsn_uniconf` | Universal configuration daemon for Yang, provides APIs for developing a client application which retreives/writes yang parameters from/to database
 LLDP         | `${SDK_INSTALL_PATH}/source/networking/tsn/tsn_lldp`    | Implementation of the IEEE 802.1 AB LLDP
 yangemb         | `<${SDK_INSTALL_PATH}>/source/networking/tsn/license_lib`    | YangDB 1 hour limitted access for LLDP Applications.

 ## Stack Initialization

Refer \ref ENET_CPSW_TSN_GPTP. <b>Stack Initialization</b> section

## Logging

Refer \ref ENET_CPSW_TSN_GPTP. <b>Logging</b> section

## Starting uniconf and LLDP applications
Refer \ref ENET_CPSW_TSN_GPTP. <b>Starting uniconf and gPTP</b> section.

This function will start:
- The uniconf task as 1st priority task to be init
- Initial uniconf DB after uniconf is finished by uniconf runtime config or yang config file.
- After uniconf and DB initialization is finished, lldp task is able to start

## Licensing library
\cond SOC_AM62X || SOC_AM62AX || SOC_AM62DX
There is yangemb-freertos.`@VAR_SOC_NAME_LOWER`.r5f.ti-arm-clang.lib located under ``<${SDK_INSTALL_PATH}>/source/networking/tsn/tsn-stack/license_lib``,
which must be added to all `tsn-stack` application's makefile.
\endcond
\cond SOC_AM62PX
There is yangemb-freertos.`@VAR_SOC_NAME_LOWER`.wkup-r5f.ti-arm-clang.lib located under ``<${SDK_INSTALL_PATH}>/source/networking/tsn/tsn-stack/license_lib``,
which must be added to all `tsn-stack` application's makefile.
\endcond

Add ``<${SDK_INSTALL_PATH}>/source/networking/tsn/tsn-stack/license_lib`` to `LIBS_PATH_common` and yangemb lib file to `LIBS_common` flags.

The licensing library will Prevent all lldp application(s) running after 1 hour.

## LLDP Deinitialization
Refer \ref ENET_CPSW_TSN_GPTP. <b>TSN Deinitialization</b> section

# Integration
## Source integration

Refer \ref ENET_CPSW_TSN_GPTP. <b>Source integration</b> section

## Uniconf configuration

Refer \ref ENET_CPSW_TSN_GPTP. <b>Uniconf configuration</b> section

## LLDP configuration parameters
This section describes the standard Yang parameters utilized for LLDP.
The LLDP Yang structure is defined in IEEE 802.1ABcu 2021 Amendment 1: YANG Data Model.

To configure LLDP params, refer ``EnetApp_setLldpRtConfig`` which is generating per-system and per-port configurations.

For ex, to configure LLDP global timer values like tx-interval, fast init, .... User can update the table accordingly.
```
static EnetApp_DbKeyVal_IntItem_t gLldpGlobalDataInt[] =
{
    {IEEE802_DOT1AB_LLDP_MESSAGE_FAST_TX, 1, sizeof(uint32_t), YDBI_CONFIG},
    {IEEE802_DOT1AB_LLDP_MESSAGE_TX_HOLD_MULTIPLIER, 4, sizeof(uint32_t), YDBI_CONFIG},
    {IEEE802_DOT1AB_LLDP_MESSAGE_TX_INTERVAL, 30, sizeof(uint32_t), YDBI_CONFIG},
    {IEEE802_DOT1AB_LLDP_REINIT_DELAY, 2, sizeof(uint32_t), YDBI_CONFIG},
    {IEEE802_DOT1AB_LLDP_TX_CREDIT_MAX, 5, sizeof(uint32_t), YDBI_CONFIG},
    {IEEE802_DOT1AB_LLDP_TX_FAST_INIT, 2, sizeof(uint32_t), YDBI_CONFIG},
};
```

To update local information data which is use to build LLDPDU, User can update the table below accordingly.
```
static EnetApp_DbKeyVal_IntItem_t gLldpLocalSysDataInt[] =
{
    // '7' mean local filled, in case of more than one ports, subtype='4' MAC address cannot apply.
    {IEEE802_DOT1AB_LLDP_CHASSIS_ID_SUBTYPE, 7, sizeof(uint32_t), YDBI_STATUS}, 
    {IEEE802_DOT1AB_LLDP_SYSTEM_CAPABILITIES_SUPPORTED, 0x07FF, 2, YDBI_STATUS}, // "11111111111"
    {IEEE802_DOT1AB_LLDP_SYSTEM_CAPABILITIES_ENABLED, 0x07BB, 2, YDBI_STATUS},   // "11110111011"
};

static EnetApp_DbKeyVal_StrItem_t gLldpLocalSysDataStr[] =
{
    // local system data
    {IEEE802_DOT1AB_LLDP_CHASSIS_ID , "00-01-02-03-04-05", YDBI_STATUS}, // Just a simple string in case of subtype is 'local (7)'
    {IEEE802_DOT1AB_LLDP_SYSTEM_NAME , "tilld", YDBI_STATUS},
    {IEEE802_DOT1AB_LLDP_SYSTEM_DESCRIPTION , "tilld", YDBI_STATUS},
};

```

Current LLDP application support up to 3 destination MAC Addresses per one port. The supported destination MAC Addresses should be match with LLDP 802.1AB specification.
- Nearest bridge 01-80-C2-00-00-0E
- Nearest non-TPMR bridge 01-80-C2-00-00-03
- Nearest Customer Bridge 01-80-C2-00-00-00

To configure per-port/dest-mac information, User can update the table `gLldpPortCfgData` accordingly.
These Destination MAC Addresses are corresponding to below configuration value.

```
// In case of any field missing in local portCfgData, global value gLldpGlobalDataInt will be used
static EnetApp_LldpPortCfg_t gLldpPortCfgData[] =
{
    {
        .destMac = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e},
        .cfgKeyValInt =
        {
            {IEEE802_DOT1AB_LLDP_ADMIN_STATUS, 3, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TLVS_TX_ENABLE, 0x0F, sizeof(uint32_t), YDBI_CONFIG},
            // If PortId subtype = P_MAC_Address (3), MAC addr will be re-correct follow hw info.
            {IEEE802_DOT1AB_LLDP_PORT_ID_SUBTYPE, 3, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_FAST_TX, 2, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_TX_HOLD_MULTIPLIER, 4, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_TX_INTERVAL, 30, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_REINIT_DELAY, 2, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TX_CREDIT_MAX, 5, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TX_FAST_INIT, 2, sizeof(uint32_t), YDBI_CONFIG},
        },
        .cfgKeyValStr = 
        {
            {IEEE802_DOT1AB_LLDP_PORT_DESC, "tilld", YDBI_CONFIG},
        }
    },
    {
        .destMac = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03},
        .cfgKeyValInt =
        {
            {IEEE802_DOT1AB_LLDP_ADMIN_STATUS, 3, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TLVS_TX_ENABLE, 0x0F, sizeof(uint32_t), YDBI_CONFIG},
            // If PortId subtype = P_MAC_Address (3), MAC addr will be re-correct follow hw info.
            {IEEE802_DOT1AB_LLDP_PORT_ID_SUBTYPE, 3, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_FAST_TX, 2, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_TX_HOLD_MULTIPLIER, 4, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_TX_INTERVAL, 20, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_REINIT_DELAY, 2, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TX_CREDIT_MAX, 5, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TX_FAST_INIT, 2, sizeof(uint32_t), YDBI_CONFIG},
        },
        .cfgKeyValStr = 
        {
            {IEEE802_DOT1AB_LLDP_PORT_DESC, "tilld", YDBI_CONFIG},
        }
    },
    {
        .destMac = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00},
        .cfgKeyValInt =
        {
            {IEEE802_DOT1AB_LLDP_ADMIN_STATUS, 3, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TLVS_TX_ENABLE, 0x0F, sizeof(uint32_t), YDBI_CONFIG},
            // If PortId subtype = P_MAC_Address (3), MAC addr will be re-correct follow hw info.
            {IEEE802_DOT1AB_LLDP_PORT_ID_SUBTYPE, 3, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_FAST_TX, 2, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_TX_HOLD_MULTIPLIER, 4, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_MESSAGE_TX_INTERVAL, 25, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_REINIT_DELAY, 2, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TX_CREDIT_MAX, 5, sizeof(uint32_t), YDBI_CONFIG},
            {IEEE802_DOT1AB_LLDP_TX_FAST_INIT, 2, sizeof(uint32_t), YDBI_CONFIG},
        },
        .cfgKeyValStr = 
        {
            {IEEE802_DOT1AB_LLDP_PORT_DESC, "tilld", YDBI_CONFIG},
        }
    },
};

```
In case of User want to support only one Destination MAC address, the entry of gLldpPortCfgData can be reduced accordingly.

# See Also

\ref NETWORKING