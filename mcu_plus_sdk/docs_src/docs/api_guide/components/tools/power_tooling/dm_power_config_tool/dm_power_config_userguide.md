# DM Power Configuration Tool User Guide {#DM_POWER_CONFIG_USERGUIDE}
[TOC]

## Introduction
DM Power Configuration Tool offers a user-friendly GUI for PLL (IP Frequency) and PSC configuration for the supported SoCs. User can create multiple config for different usecase. Based on UI config, code is generated for single/multiple config.

## How to Use the GUI and Generate Output Code

#### Prerequisites

- [SysConfig](https://www.ti.com/tool/SYSCONFIG) version 1.20.0

The SysConfig should be installed in a directory with the following structure: `$(HOME_DIR)/ti/sysconfig_$(SYSCONFIG_VERSION)`, where `HOME_DIR` represents your home directory and `SYSCONFIG_VERSION` is the version of SysConfig.

If you wish to use a different version of SysConfig or change the installation path, simply update the `SYSCONFIG_VERSION` or `SYSCFG_PATH` in the Makefile.

### For MCU+ SDK

-# **Open the DM Power Config Tool GUI**

   Run the following command to open GUI (For Hello World Example):

   `make -s -C examples/hello_world/j722s-evm/wkup-r5fss0-0_freertos/ti-arm-clang/ syscfg-gui`

-# **Configure PLL and PSC**

    **IP Frequency (MHz)**

    This tab presents a list of key IPs within the SoC and enables users to configure their frequencies. To set the desired frequency, select the appropriate value and check the "Custom Clock Enable" checkbox. If this checkbox is disabled, the IP frequency will not be configured.
    \imageStyle{dm_power_config_tool_mcuplussdk_pll.png,width:45%} \image html dm_power_config_tool_mcuplussdk_pll.png

    **Notes:**
    - The IP frequency settings depend on the configurations that the Device Manager (DM) can support.
    - The frequency values displayed in the tool are in MHz and should be specified with up to six decimal places when a fractional part is required. This level of precision        ensures that the desired frequency is accurately represented.
    - Some IPs may be non-editable, as they cannot be configured by the user.

   **PSC Config**

   This tab lists IPs that can be enabled or disabled using the checkbox.
        \imageStyle{dm_power_config_tool_mcuplussdk_psc.png,width:45%} \image html dm_power_config_tool_mcuplussdk_psc.png

   The configuration settings made in the GUI will be saved in the file `examples/hello_world/j722s-evm/wkup-r5fss0-0_nortos/example.syscfg`, . This file is utilized for generating the output code.


-# **Generate Code Based on the Configuration**

   After configuring the desired settings in the GUI, execute the following command to generate the code:

   `make -s -C examples/hello_world/j722s-evm/wkup-r5fss0-0_freertos/ti-arm-clang/ syscfg`

   The generated code is included in the `ti_power_clock_cfg.c` file.

-# **Build Example**

   `make -s -C examples/hello_world/j722s-evm/wkup-r5fss0-0_freertos/ti-arm-clang/`

 For more information about the SDK SysConfig and how to use it, please refer to the [MCU+ Academy](https://dev.ti.com/tirex/explore/node?node=A__ADGqnjXCjlsfY7kBsbtymg__com.ti.MCU_PLUS_ACADEMY_AM64X__n6QeJt5__LATEST).



### User Configuration

Users can customize the list of IPs and PSCs supported by the tool. Refer to \subpage CUSTOM_CONFIG for detailed instructions on how to add or remove IPs and PSCs from the tool.

### Supported SoCs
- J784S4
- J722S/J7AEN
- J721S2
