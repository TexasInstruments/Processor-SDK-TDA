# DM Power Analysis Tool {#DM_POWER_ANALYSIS_TOOL}

[TOC]

## Introduction

The DM Power Analysis tool generates a snapshot of power and clock management IPs for Jacinto SoCs. This tool helps in understanding device power utilization by providing critical information about PLL, PSC, and clock configuration.
The generated output includes:

* Clock tree
* PLL configuration
* PSC configuration
* PET clock table

### PLL

PLLs (Phase-Locked Loops) are responsible for the clock frequencies of IPs within their domain. Each PLL contains a fixed number of dividers called HSDIVs.
There are separate PLLs for the Main and the MCU domains. There are two possibilities to change a particular input clock frequency of an IP:

* Change VCO frequency (Output of PLL)
* Change HSDIV frequency (Output of HSDIV)

To obtain the required frequency for the particular clock, the VCO and/or HSDIV frequency can be adjusted. Understanding the current configuration of PLLs and their HSDIVs helps optimize power usage in terms of clock frequency. For example, the data from this tool enables you to identify issues such as whether any IP clock is not needed at all, or the frequency is more than what is required.

### PSC

PSC(Power Sleep Controllers) control overall device power by turning off unused power domains and gating off clocks to individual peripherals and modules.
Each PSC contains a list of PDs (Power Domains), and each PSC includes a list of LPSCs (Local Power Sleep Controllers) connected to it. Each LPSC controls power for one or more modules/IPs.
To turn off power to a particular module, we can disable the LPSC controlling the module, provided that all devices connected to that LPSC are also disabled.
Enabling modules/IPs that are not necessary for a particular use case might increase the power consumption of the entire device.
Hence, the information from this tool helps you identify the modules/IPs that are enabled and scrutinize the power utilization.

## Features

* Generates clock tree, PLLs configuration, and PSC configuration for [Supported SoCs](#supported-socs-dm)
* Generates individual PLL/PSC/device clock config and also SoC level tree, PSC and PLL config.
* Generates IP frequency details as per Power Estimation Tool(PET).

## Requirements

To run the tool you need:

* J7xx EVM

##  Build

1. To utilize the DM Power Analysis tool, pdm_utils library should be linked while building the application.
   this library is already part of mcu_plus_sdk top level library.
  * `make -sj libs DEVICE=<evm_name>` (For J722S)
  * `make BOARD=<board_name> CORE=<core> pdm_utils -sj `(For J784S4/J721S2/J721e/J7200)

##  Usage

1. In the application, whenever needed, call the PDM utils APIs to get the current state of the device.
2. The following are the APIs provided by PDM utils
   * `PdmUtils_getPllDataBook(void *buf, uint32_t size);`/* Provides PLL settings status */
   * `PdmUtils_getPscDataBook(void *buf, uint32_t size);`/* Provides PSC settings status */
   * `PdmUtils_getPetClockBook(void *buf, uint32_t size);` /* Provides PET clock table */
   * `PdmUtils_getSocClockTree(void *buf, uint32_t size);` /* Provides SoC clock tree */
3. Application needs to provide the memory to obtain the data object from the above APIs

## Outputs

### PLL status output
All the PLLs status and configuration of device are provided.

```
========================================================================================================================
|  PLL Name   | status  |         config        |     postdiv    | VCO       | Num of |  HSDIV  |  status  | Output    |
|             |         |                       |                | Freq(MHz) | HSDIVs |         |          | freq(MHz) |
========================================================================================================================
|MAIN_PLL_0   |ENABLED  |M=80, FracM=0          |ENABLED, M2=2   |2000       |10      | HSDIV 0 |ENABLED   |500        |
|             |         |                       |                |           |        | HSDIV 1 |ENABLED   |200        |
|             |         |                       |                |           |        | HSDIV 2 |ENABLED   |200        |
|             |         |                       |                |           |        | HSDIV 3 |ENABLED   |133        |
|             |         |                       |                |           |        | HSDIV 4 |ENABLED   |80         |
|             |         |                       |                |           |        | HSDIV 5 |ENABLED   |200        |
|             |         |                       |                |           |        | HSDIV 6 |ENABLED   |200        |
|             |         |                       |                |           |        | HSDIV 7 |ENABLED   |200        |
|             |         |                       |                |           |        | HSDIV 8 |ENABLED   |50         |
|             |         |                       |                |           |        | HSDIV 9 |ENABLED   |100        |
------------------------------------------------------------------------------------------------------------------------

```
* PLL Name: Name of the PLL
* status  : status whether PLL/HSDIV is enabled or disabled.
* config  : Integer divider and fractional divider values.
* Num of HSDIVs: Integer value specifying the number of high-speed dividers.
* VCO freq: VCO frequency of the PLL
* output freq: Output frequency of the HSDIV

### PSC Status output on the UART console
This output represents the status of Power State Controllers (PSCs).

### Hierarchy
The hierarchy is as follows:

* PSC: Device contains multiple PSCs (e.g. J7_WKUP_PSC_WRAP_WKUP_0).
* PD: Each PSC contains multiple Power Domains (PDs).
* LPSC: Each PD contains multiple Low-Power State Controllers (LPSCs).
* IP: Each LPSC is connected to a list of Intellectual Properties (IPs).
### Properties
* PSC: Top-level key representing a PSC, containing:
    + PD: An object with multiple PD properties.
* PD: An object representing a Power Domain, containing:
     + LPSC: An object with multiple LPSC properties.
* LPSC: An object representing a Local Power & State Controller, containing:
    + status: A string indicating the status of the LPSC (ENABLED or DISABLED).
    + controlled_ip_instances: An array of strings representing the IPs connected to the LPSC

```
----------------PSC INFO STARTS-----------------
====================================================================================================================================================================
           PSC Name        |     Power Domain    |   PD status   |             LPSC name          |   LPSC Status   |                 Connected IPs                |
====================================================================================================================================================================
  J7_WKUP_PSC_WRAP_WKUP_0  | PD_GP_CORE_CTL_MCU  |      ENABLED  |             LPSC_MCU_ALWAYSON  |        ENABLED  |                 WKUP_MCU_GPIOMUX_INTROUTER0  |
                           |                     |               |                                |                 |                                    MCU_DCC0  |
                           |                     |               |                                |                 |                                    MCU_DCC1  |
                           |                     |               |                                |                 |                                   WKUP_ESM0  |
                           |                     |               |                                |                 |                                   MCU_GPIO0  |
                           |                     |               |             LPSC_MAIN2MCU_ISO  |        ENABLED  |                                 MAIN2MCU_VD  |
                           |                     |               |               LPSC_DM2MCU_ISO  |        ENABLED  |                                              |
                           |                     |               |              LPSC_DM2SAFE_ISO  |        ENABLED  |                                              |
                           |                     |               |               LPSC_MCU2DM_ISO  |        ENABLED  |                                              |
                           |                     |               |                 LPSC_MCU_TEST  |       DISABLED  |                                              |
-------------------------------------------------------------------------------------------------------------------------------------------------------------------

```
### PET clock table output

A table with IP clock frequencies corresponding to the PET tool

```
========================================
|       Frequency of selected IPs      |
========================================
|       IP Name           |  Freq(MHz) |
========================================
|               MAIN SMS 0|         400|
|             WKUP R5FSS 0|         800|
|              MCU R5FSS 0|         800|
|             MAIN R5FSS 0|         800|
|             MAIN A53SS 0|        1250|
|             MAIN C71SS 0|        1000|
|             MAIN C71SS 1|        1000|
|                   VPAC 0|         600|
|                  DMPAC 0|         428|
|               MAIN GPU 0|           0|
|  Video Encoder/Decoder 0|         500|
|      MAIN JPEG Encoder 0|         250|
|            LPDDR4 EMIF 0|         933|
|           MCU Domain CLK|         500|
|Device Manager Domain CLK|         400|
|          Main Domain CLK|         500|
|           HSM Domain CLK|         400|
========================================

```
1. This table shows the current frequencies of the IPs. The list of IPs represented in the table are same as in the PET tool.
2. This table helps calculate Power Estimation with PET tool since this table is directly correlates to the PET tool inputs.

### Clock ip tree output

```
--------------------------------CLOCK TREE STARTS--------------------------------
MODULE DBGSUSPENDROUTER0
  Number of clocks for device : 1
	Module clock   0:SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK
	Clock tree      :GLUELOGIC_HFOSC0_CLKOUT(25 MHz) -> PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK(2000 MHz) -> HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK(500 MHz) -> SAM62_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK(500 MHz) -> SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK(125 MHz)
------------------------------------------------------------------------
```
1. The output contains module name, number of clocks connected to the module, and clock tree for each module clock.

..
## Supported SoCs {#supported-socs-dm}

The tool supports the following SoCs:

- j784s4
- j721s2
- j721e
- j7200
- j722s

## Limitations

1. The following features are not available on cores other than mcu1_0/WKUP_R5 core

- Clock IP tree 
- Correlation between PSC/PD/LPSC and connected devices.(Refer to "Power Management Device-Level Layout" table in SoC TRM)

2. For J721e & J7200 SoCs, PET clock table is not available. 

## Known Issues
None
