# Using Fast-xSPI Bootmode {#FAST_XSPI_BOOTMODE_GUIDE}

[TOC]

## Introduction

The Fast-xSPI boot mode is an extension of the xSPI boot mode where the OSPI PHY will be tuned
in order to set the frequency of operation to 100MHz / 133MHz for 8D-8D-8D mode. Flash needs to support
SFDP.

To support Fast xSPI boot, ROM expects the tuning configuration in the last sector of
flash which is determined by the flash device configuration. Absence of the configuration will cause ROM to switch the non-PHY based/standard xSPI boot.

Get the flash memory density (flash memory size S) and largest erase sector size
(E bytes) from the BFPT table and calculates the beginning of last sector as E bytes from the end
of flash. We call this location as start of the configuration data (C0 = S - E)

The sample binary ' OSPI_offline133Mhz ' with tuning configuration for 133 MHz is provided in the prebuilt folder ' ${SDK_INSTALL_DIR}/tools/boot/sbl_prebuilt/@VAR_BOARD_NAME_LOWER '
Flash this binary in the last sector of flash and change the bootmode pins to Fast-xSPI Bootmode to make the ROM boot in Fast-xSPI mode. For example, on EVM with the Infineon/Cypress s28hs512t device of size 512 Mb the parameters needs to be flashed at offset 0x03FC0000.

\image html boot_pins_fastxSPI.png "FAST-xSPI BOOTMODE" width=25%

### Skip Tuning by the SBL

To make use of the OSPI Phy configuration done by ROM and avoid the additional overhead of ospi phy tuning by the SBL, make sure to check the ' Skip OSPI Tuning ' in the Sysconfig of SBL

\image html skipTuning.png "OSPI SysConfig Module GUI" width=50%

