# Bootloader Test Example {#EXAMPLES_BOOTLOADER_TEST}

[TOC]

# Introduction

This is a bootloader test application which demonstrates booting an encrypted application signed with certificate generated for inplace auth type .The example is supported only on HS-SE device.
In this example the image information such as header, image residing address after auth and decryption and image size are obtained as a response from Sciclient_procBootAuthAndStart api.
In this example the appimage is copied from flash to ddr memory to do authentication and decryption, as decryption can't be done in flash memory.

# Supported Combinations {#EXAMPLES_BOOTLOADER_TEST_COMBOS}

\cond SOC_AM62AX

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | r5fss0-0 nortos
 Toolchain      | ti-arm-clang
 Boards         | @VAR_BOARD_NAME_LOWER
 Example folder | examples/security/boootloader_test

\endcond

\cond SOC_AM62PX

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | wkup-r5fss0-0 nortos
 Toolchain      | ti-arm-clang
 Boards         | @VAR_BOARD_NAME_LOWER
 Example folder | examples/security/boootloader_test

\endcond

# Steps to Run the Example

## Build the example

Since this is a bootloader application, the example will be run every time you boot an application using this example. It is run from a OSPI boot media  unlike other examples which are usually loaded with CCS. Nevertheless, you can build this example like you do for the others using makefile or build it via CCS by importing as a project.

\note This example runs on HS-SE devices so the DEVICE_TYPE in {SDK_PATH}/devconfig/devconfig.mak must be set to HS.

- **When using CCS projects to build**, import the CCS project for the required combination
  and build it using the CCS project menu (see \ref CCS_PROJECTS_PAGE).
- **When using makefiles to build**, note the required combination and build using
  make command (see \ref MAKEFILE_BUILD_PAGE)
- Refer to the page \ref BASIC_STEPS_TO_FLASH_FILES to flash this bootloader along with the application to boot.

## Run the example

- This example is the SBL which needs to be flashed on the EVM flash, along with application images for MCU R5 Appimage.
\note For HS-SE device, use **default_security_bootloader_test_hs.cfg** as the cfg file.

- There is a default flash config file as shown below which flashes this SBL and the \ref EXAMPLES_MCU_CORE_TEST applications

        ${SDK_INSTALL_PATH}/examples/security/bootloader_test/@VAR_BOARD_NAME_LOWER/default_security_bootloader_test_hs.cfg

- Make sure mcu_core_test_app application is built before running the flash script. (see \ref EXAMPLES_MCU_CORE_TEST )

- To flash to the EVM, refer to \ref GETTING_STARTED_FLASH . Only when giving the flash config file, point to the `default_security_bootloader_test_hs.cfg` shown above.

- Example, assuming SDK is installed at `C:/ti/mcu_plus_sdk` and this example and mcu_core_test_app application is built using makefiles, in Windows,

        cd C:/ti/mcu_plus_sdk/tools/boot
        python uart_uniflash.py -p COM13 --cfg=C:/ti/mcu_plus_sdk/examples/security/bootloader_test/@VAR_BOARD_NAME_LOWER/default_security_bootloader_test_hs.cfg

- If Linux PC is used, assuming SDK is installed at `~/ti/mcu_plus_sdk`

        cd ~/ti/mcu_plus_sdk/tools/boot
        python uart_uniflash.py -p /dev/ttyUSB0 --cfg=~/ti/mcu_plus_sdk/examples/security/bootloader_test/@VAR_BOARD_NAME_LOWER/default_security_bootloader_test_hs.cfg

\cond SOC_AM62AX
- Boot the EVM in OSPI NAND boot mode.
\endcond

\cond SOC_AM62PX
- Boot the EVM in OSPI NOR boot mode.
\endcond

# Sample Output

Shown below is a sample output when the application is run,

\code
Image residing at address: 0x84000000
Image size: 0x8970
\endcode