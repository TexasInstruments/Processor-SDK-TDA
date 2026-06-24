# Release Notes 11.02.01 {#RELEASE_NOTES_11_02_01_PAGE}

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

 Feature                                                                   | Module
---------------------------------------------------------------------------|-----------------------------------
 Support LPM (Low Power Mode) in IPC Firmware                              | IPC


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


## Fixed Issues

ID        | Head Line                                                                        | Module   | Affected Versions      | Affected Platforms
----------|----------------------------------------------------------------------------------|----------|------------------------|-------------------
PDK-19904 | IPC FW should support lpm on j722s                                               | IPC      | PROCESSOR_SDK_11.02.00 | J722S
PDK-20199 | csirx and csitx 1 sec delay used instead of 1ms                                  | CSI2RX, CSI2TX | PROCESSOR_SDK_11.02.00 | J722S
PDK-19247 | [CSIRX]: driver resets all streams in case of header error                       | CSI2RX   | PROCESSOR_SDK_11.02.00 | J722S


## Errata Fixes

ID        | Head Line                                                                        | Module   | Affected Versions      | Affected Platforms
----------|----------------------------------------------------------------------------------|----------|------------------------|-------------------
PDK-17407 | [Errata] J722S: ROM boot fails for large file sizes                              | SBL      | PROCESSOR_SDK_10.01.00 | J722S
PDK-11220 | [Errata] USART: Erroneous triggering of timeout interrupt                        | UART     | PROCESSOR_SDK_08.02.00 | J722S


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

