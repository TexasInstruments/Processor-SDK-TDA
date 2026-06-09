# Release Notes 11.02.00 {#RELEASE_NOTES_11_02_00_PAGE}

[TOC]

\attention Also refer to individual module pages for more details on each feature, unsupported features, important usage guidelines.
\parblock
\note The examples will show usage of SW modules and APIs on a specific CPU instance and OS combination. \n
      Unless explicitly noted otherwise, the SW modules would work in FreeRTOS in R5F and C75 cores and no-RTOS in C75 cores.\n
\endparblock
\parblock
\endparblock

## Device and Validation Information

SOC    | Supported CPUs                              | EVM                                                    | Host PC
-------|---------------------------------------------|--------------------------------------------------------|-----------------------------------
J722S  | MCU R5F, WKUP R5F, MAIN R5F, C75SS0, C75SS1 | @VAR_BOARD_NAME (referred to as j722s-evm in code)     | Ubuntu 22.04 64b


## Features Added in This Release

 Feature                                                                                            | Module
----------------------------------------------------------------------------------------------------|------------
 TI ARM Clang compiler upgrade to 4.0.4                                                             | BUILD
 Support for FreeRTOS LTS 202406.01 LTS                                                             | DPL
 FreeRTOS port shall include a default exception handler to aid in exception debug on C7x           | DPL
 SCICLIENT LLD is compliant to safety qualification approach (ASIL D)                               | DM
 SafeRTOS DPL + Portable layer (R5, C7x) is compliant to safety qualification approach (ASIL B)     | DPL
 FVID2 LLD is compliant to safety qualification approach (ASIL B)                                   | FVID2
 IPC LLD is compliant to safety qualification approach (ASIL B)                                     | IPC
 I2C LLD is compliant to safety qualification approach (ASIL B)                                     | I2C
 UDMA LLD is compliant to safety qualification approach (ASIL D)                                    | UDMA

## Dependent Tools and Compiler Information

Tools                   | Supported CPUs                               | Version
------------------------|----------------------------------------------|----------------------------------------------
Code Composer Studio    | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1  | @VAR_CCS_VERSION
SysConfig               | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1  | @VAR_SYSCFG_VERSION, build @VAR_SYSCFG_BUILD
TI ARM CLANG            | MCU-R5F, WKUP-R5F, MAIN-R5F                  | @VAR_TI_ARM_CLANG_VERSION
TI CGT                  | C75SS0, C75SS1                               | @VAR_TI_C7000_CGT_VERSION
FreeRTOS Kernel         | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1  | @VAR_FREERTOS_KERNEL_VERSION

## Compiler options used for build

Core             | Compiler Options
-----------------|----------------------------------------------------------------------------------------------------------------------------------------------
R5F              | -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mthumb -Wall -Werror -g -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function  -Os
C7x              | -mv7524 --abi=eabi -q -mo -pden -pds=238 -pds=880 -pds1110 --endian=little --disable_inlining -ea.se71 --emit_warnings_as_errors --diag_suppress=770 --diag_suppress=69 --diag_suppress=70 --advice:performance=none  --symdebug:none -Dxdc_target_name__=C71 --opt_level=3

## Key Features

### OS Kernel

OS              | Supported CPUs                              | SysConfig Support
----------------|---------------------------------------------|-------------------
FreeRTOS Kernel | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | NA
SafeRTOS Kernel | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | NA
NO RTOS         | MCU-R5F, WKUP-R5F, MAIN-R5F                 | NA

### Driver Porting Layer (DPL)

Module            | Supported CPUs                              | SysConfig Support | OS support(NoRTOS only on R5F)
------------------|---------------------------------------------|-------------------|-------------------------------
Address Translate | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS
Cache             | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS
Clock             | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS
CycleCounter      | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | NA                | SafeRTOS,FreeRTOS, NORTOS
Debug             | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS
Heap              | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | NA                | FreeRTOS,NORTOS
Hwi               | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS
MPU               | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS
Semaphore         | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | NA                | SafeRTOS,FreeRTOS, NORTOS
Task              | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | NA                | SafeRTOS,FreeRTOS
Timer             | MCU-R5F, WKUP-R5F, MAIN-R5F, C75SS0, C75SS1 | YES               | SafeRTOS,FreeRTOS, NORTOS

### Secondary Bootloader (SBL)

SBL Mode  | Supported CPUs | SysConfig Support | PHY Support | DMA Support | OS support | HLOS Boot(Linux/QNX/U-BOOT)
----------|----------------|-------------------|-------------|-------------|------------|------------------------------
OSPI NOR  | WKUP-R5F       | YES               | YES         |   YES       | NORTOS     | Yes
OSPI NAND | WKUP-R5F       | YES               | YES         |   YES       | NORTOS     | Yes
EMMC      | WKUP-R5F       | YES               | NA          |   NA        | NORTOS     | Yes
UART      | WKUP-R5F       | YES               | NA          |   No        | NORTOS     | No
MMCSD     | WKUP-R5F       | YES               | NA          |   Yes       | NORTOS     | Yes

### Networking

Module                      | Supported CPUs | SysConfig Support | OS Support  | Key features tested                                                                                                                                                                    | Key features not tested
----------------------------|----------------|-------------------|-------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------
Ethernet Firmware                        | MAIN-R5F       | YES               | FreeRTOS    | TCP/UDP IP networking stack with and without checksum offload enabled, TCP/UDP IP networking stack with server and client functionality, basic Socket APIs, DHCP, ping, TCP iperf, LwIP bridge, shared memory driver,gPTP IEEE 802.1 AS-2020 compliant gPTP stack  | Other LwIP features
Ethernet driver (ENET)      | MAIN-R5F      | YES               | FreeRTOS    | Ethernet as port using CPSW,  MAC loopback  | RMII mode

### SOC Device Drivers

<table>
    <tr>
        <th>Peripheral</th>
        <th>Domain</th>
        <th>Supported CPUs</th>
        <th>SysConfig Support</th>
    </tr>
    <tr>
        <td>DDR</td>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>DSS</td>
        <td>Main</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>GPIO</td>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Main</td>
        <td>MAIN-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>I2C </td>
        <td>Main</td>
        <td>MCU-R5F, WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F, WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>MCU-R5F, WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>IPC </td>
        <td>Main</td>
        <td>MAIN-R5F, C75SS0, C75SS1</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>UDMA </td>
        <td>Main</td>
        <td>MAIN-R5F, C75SS0, C75SS1</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=1>CSI-Rx </td>
        <td>Main</td>
        <td>MAIN-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=1>CSI-Tx </td>
        <td>Main</td>
        <td>MAIN-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>MCSPI </td>
        <td>Main</td>
        <td>MAIN-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCAN</td>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>Pinmux</td>
        <td>Main</td>
        <td>MAIN-R5F, C75SS0, C75SS1</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>SOC</td>
        <td>NA</td>
        <td>MCU-R5F, WKUP-R5F</td>
        <td>YES</td>
    </tr>
     <tr>
        <td rowspan=3>SCIClient</td>
        <td>Main</td>
        <td>MAIN-R5F, C75SS0, C75SS1</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td rowspan=3>UART</td>
        <td>Main</td>
        <td>MCU-R5F, WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>MCU</td>
        <td>MCU-R5F, WKUP-R5F</td>
        <td>YES</td>
    </tr>
    <tr>
        <td>Wakeup</td>
        <td>WKUP-R5F</td>
        <td>YES</td>
    </tr>
</table>


## Upgrade and Compatibility Information

This section lists changes which could affect user applications developed using older SDK versions.
Read this carefully to see if you need to do any changes in your existing application when migrating to this SDK version relative to
previous SDK version. Also refer to older SDK version release notes to see changes in
earlier SDKs.

<table>
<tr>
    <th> Module
    <th> Change Summary
    <th> Details
</tr>
<tr>
    <td> I2C
    <td> I2C_Transaction structure updated
    <td> Structure updated to align with modern Controller/Target terminology and support memory-mapped transactions: <br/>
         - **Renamed Fields:** `slaveAddress` → `targetAddress`, `masterMode` → `controllerMode` <br/>
         - **Type Changes:** `writeBuf` is no longer `const` <br/>
         - **New Features:** `memTxnEnable` and `memTransaction` pointer for memory read/write operations <br/>
         - **Status Reporting:** Added `status` field to track transaction results <br/>
         Applications using I2C driver require updates to use new field names. <br/>
         This change is done as per the naming recommendation.
</tr>
<tr>
    <td> Build
    <td> In Boot Image Generation, authentication type used in appimage sigining is updated.
    <td> authtype used in appimage sigining is changed from 2 to 0. load address should be provided explicitly where the app image should be copied during authentication. <br/>
        By default SBL expects the load address to be at address `0xC2000000`
        Example: `--authtype 0 --loadaddr 0xC2000000` in `appimage_x509_cert_gen.py` <br/>
        Auto-generated makefiles are updated; verify custom boot scripts if applicable.
        The `authtype` parameter in boot image signing specifies whether the binary should be copied to the specified load address during authentication. <br/>
        The valid values for this field and their interpretation are described below.
         - authtype 0: Image is copied to load address
         - authtype 1: In-place authentication binary is not moved
         - authtype 2: In place operation variant. Binary is moved to the beginning of the buffer
</tr>
<tr>
    <td> Build
    <td> wake up r5 binary path in all mcu sdk examples is updated.
    <td> In all example makefiles, `SCISERVER_IMAGE_PATH` is updated to directly point to the sciclient example directory built for the Wake-up R5F core. <br/>
    The prebuilt binary image is also present at `${MCUSDK_INSTALL_DIR}/tools/sysfw` same as the previous release.
</tr>
<tr>
    <td> Build
    <td> command line parameters for generating linux app image is updated.
    <td> Linux application image generation for FASTBOOT_LINUX now uses **FALCON_MODE** parameter. When `FALCON_MODE=1` is set, the kernel (Image) and device tree (DTB) are loaded directly instead of using SPL, bypassing the secondary bootloader. <br/>
         **Example:** `make FALCON_MODE=1` in `tools/boot/linuxAppimageGen/` generates a boot image that loads the Linux kernel directly. <br/>
         This parameter affects the boot image composition and loading behavior during Linux image initialization.
</tr>
<tr>
    <td> Build
    <td> Firmware binary naming: tifs → sysfw
    <td> All references updated from `tifs-*.bin` to `sysfw-*.bin` across boot scripts, makefiles, and image generation tools. <br/>
         This affects build configurations and firmware deployment paths.
</tr>
<tr>
    <td> Build
    <td> SysConfig outputs regenerated
    <td> All SysConfig-generated `.meta` templates have been regenerated: <br/>
         GPIO, I2C (including LLD templates), UART, DSS (v0/v1), UDMA (v0/v1), MMCSD, EEPROM
         Regenerate project configurations from SysConfig to apply updates.
</tr>
<tr>
    <td> Build
    <td> Removed module_cfg.h header
    <td> The `module_cfg.h` file has been removed from `${SDK_INSTALL_DIR}/source/drivers`. <br/>
         Update build configurations if this header was previously included in custom applications.
</tr>
</table>

## Fixed Issues

ID        | Head Line                                                                        | Module   | Affected Versions      | Affected Platforms
----------|----------------------------------------------------------------------------------|----------|------------------------|-------------------
PDK-18718 | Board_detectBoard does properly detect Fusion2                                   | BOARD    | PROCESSOR_SDK_11.01.00 | J722S
PDK-17912 | Incorrect macro SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_LANE_CONFIG                  | CSI2RX   | PROCESSOR_SDK_11.00.00 | J722S
PDK-18992 | Random c7x ipc failure when configuring CLEC                                     | IPC      | PROCESSOR_SDK_11.01.00 | J722S
PDK-19333 | Define extended_system_pre_init function as weak attribute in system_pre_init    | SBL      | PROCESSOR_SDK_11.01.00 | J722S
PDK-19687 | J722S memory map conflicts between sysconfig and gen_linker                      | IPC      | PROCESSOR_SDK_11.01.00 | J722S


## Known Issues

NA


## Limitations

<table>
<tr>
    <th> S.No
    <th> Head Line
    <th> Module
</tr>
<tr>
    <td> 1
    <td> The **ROM** startup model for runtime initializations in TI ARM CLANG is not supported/tested in the SDK
    <td> NA
</tr>
<tr>
    <td> 2
    <td> CCS can not be used in No Boot mode.
    <td> Use SBL NULL to side load binaries and debug using CCS.
</tr>
<tr>
    <td> 3
    <td> The EVM is limited to only one MAC address in the EEPROM, applications requiring multiple MAC addresses should enable and configure manual MAC address entry in Sysconfig.
    <td> Networking
</tr>
</table>
</table>

