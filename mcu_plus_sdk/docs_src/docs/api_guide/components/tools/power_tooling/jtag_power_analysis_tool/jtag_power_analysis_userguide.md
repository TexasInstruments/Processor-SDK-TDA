# JTAG Power Analysis Tool {#JTAG_POWER_ANALYSIS_USERGUIDE}

[TOC]

## Introduction

The JTAG power analysis tool generates a snapshot of clock management IPs for Jacinto SoCs. The generated output includes:

- PLL status and settings
- PSC status and settings
- Clock tree
- Junction Temperature (for CCS20 Debugger only)

The tool is designed to work with the [Supported Debuggers](#supported-debuggers).

## Features

- Generates/Prints PLLs status/settings, PSC status/settings, clock tree and SoC junction temperature for supported [SOCs](#supported-socs)
- Saves the output of tool in json format
- Provides a user configuration file to selectively print the PLL and LPSC status for a subset of IPs present on the SOC, including the Power Estimation Tool IPs/modules.

## Requirements

To generate the PLLs status/settings, PSC status/settings and clock tree for a SoC, you need:

- J7xx EVM
- Debugging Software (See [Usage](#usage) of desired debugger for specific requirements).

## Installation

1. Install node dependencies
    In `jtag_power_analysis_tool/` run `npm ci` or `npm install`.

## Usage {#usage}

The JTAG Power Analysis Tool operates in a two-step process:

1. Initially, a debugger script is executed to read the MMRs (Memory-Mapped Registers) and generate a list of key-value pairs, representing the MMR addresses and their corresponding values.

2. The tool then utilizes this list of key-value pairs to generate comprehensive outputs, including PLL status and settings, PSC status and settings and clock tree.

The modular structure of the JTAG Power Analysis Tool allows for easy integration with any debugger. This independence from the debugger enables the tool to be used with any debugger, making it adaptable to the needs of the user.

### Running with Code Composert Studio (CCS)

Please refer to \subpage JTAG_CCS for instructions to run this tool with CCS.

### Running with Trace32 PowerView

Please refer to \subpage JTAG_TRACE32 for instructions to run this tool with Trace32.

## Output Formats {#output_formats}

### JSON Files

The output json files are saved in `jtag_power_analysis_tool/output/`. Sample output file can be found in `jtag_power_analysis_tool/docs/sample_output/<soc>`.

Following is a snapshot of the JSON output file for the j722s SoC running `hello_world.release.appimage.hs_fs` app with `sbl_sd.release.hs_fs.tiimage` SBL .

#### PLL

##### Description

The `plls_status_<soc>.json` is a JSON file that represents the PLL clock configuration. It contains the status of each PLL in the SoC. The file is organized as a JSON object, where each key represents a specific PLL and the corresponding value is another JSON object that contains the PLL status.

##### Clock Configuration Key

- isEnable: Boolean value indicating whether the PLL is enabled.
- numHsdiv: Integer value specifying the number of high-speed dividers.
- freqOut: Object with foutVco and foutPostDiv properties.
- hsdivOut: Array of objects with idx, isEnable, freqOut, and hsDiv properties.
- settings: Object with postDiv1, postDiv2, vcoFreqMul, M, fracM, and isPostDivEn properties.

```
    "PLLFRAC2_SSMOD_16FFT_MAIN_14": {
      "isEnable": 1,
      "numHsdiv": 2,
      "freqOut": {
        "foutVco": 2000,
        "foutPostDiv": 2000
      },
      "hsdivOut": [
        {
          "idx": 0,
          "isEnable": 1,
          "freqOut": 1000,
          "hsDiv": 2
        },
        {
          "idx": 1,
          "isEnable": 1,
          "freqOut": 1000,
          "hsDiv": 2
        }
      ],
      "settings": {
        "postDiv1": 1,
        "postDiv2": 1,
        "vcoFreqMul": 104.16666668653488,
        "M": 104,
        "fracM": 2796203,
        "isPostDivEn": 1
      }
    }
```

#### PSC

##### Description

The `psc_status_<soc>.json` is a hierarchical JSON object that represents the status of PSC.

##### Hierarchy
The hierarchy is as follows:

- PSC: Top-level keys representing individual PSCs (e.g. WKUP_PSC0).
- PD: Each PSC contains multiple Power Domains (PDs).
- LPSC: Each PD contains multiple LPSCs.
- IP: Each LPSC is connected to a list of IPs.

##### Properties

- PSC: Top-level key representing a PSC, containing multiple:
    - PD: An object with multiple PD properties.
- PD: An object representing a Power Domain, containing multiple:
    - LPSC: An object with multiple LPSC properties.
- LPSC: An object representing a Local Power & State Controller, containing:
    - status: A string indicating the status of the LPSC (ENABLED or DISABLED).
    - controlled_ip_instances: An array of strings representing the IPs connected to the LPSC

```
"WKUP_PSC0": {
      "PD_wkup": {
        "lpsc_list": {
          "LPSC_wkup_gpio": {
            "status": "ENABLED",
            "controlled_ip_instances": [
              "WKUP_GPIO0",
              "WKUP_GPIO1",
              "WKUP_I2C0",
              "WKUP_UART0"
            ]
          },
          "LPSC_mcu_ospi_0": {
            "status": "ENABLED",
            "controlled_ip_instances": [
              "MCU_FSS0_OSPI_0"
            ]
          }
        },
        "status": "ENABLED"
      },
      "PD_mcu_pulsar": {
        "lpsc_list": {
          "LPSC_mcu_r5_0": {
            "status": "ENABLED",
            "controlled_ip_instances": [
              "MCU_R5FSS0_CORE0",
              "MCU_RTI0"
            ]
          }
        "status": "ENABLED"
      }
    }
  }
```

#### Clock Tree

The `clock_tree_<soc>.json` is a hierarchical JSON object.
Each key in the clock tree represents a clock IP instance.
Each clock IP instance has multiple input clocks.
For each input clock, the JSON array lists the clock and its frequency starting from the root clock.

```
{
"IP1":[
[{clk: "clk0", freq_mhz: 1000}, {clk: "clk1", freq_mhz: 200}, {clk: "clk2", freq_mhz: 100}],
[{clk: "clk0", freq_mhz: 1000}, {clk: "clk1", freq_mhz: 200}, {clk: "clk2", freq_mhz: 50}],
],
"IP2":[
[{clk: "clk3", freq_mhz: 2000}, {clk: "clk4", freq_mhz: 400}, {clk: "clk5", freq_mhz: 400}],
[{clk: "clk3", freq_mhz: 2000}, {clk: "clk4", freq_mhz: 500}, {clk: "clk5", freq_mhz: 250}],
]
}
```

For example, the `J722S_DEV_GPU0` key in j722s represents the clock paths for the GPU0 IP. The array contains two sub arrays(GPU_PLL_CLK and PLL_CTRL_CLK), each representing a clock path. Each path includes the clock and its frequency.

```
        "J722S_DEV_GPU0": [
      [
        {
          "clk": "CLK_J722S_GLUELOGIC_HFOSC0_CLKOUT",
          "freq_mhz": 25
        },
        {
          "clk": "CLK_J722S_PLLFRACF2_SSMOD_16FFT_MAIN_6_FOUTVCOP_CLK",
          "freq_mhz": 2160
        },
        {
          "clk": "CLK_J722S_HSDIV0_16FFT_MAIN_6_HSDIVOUT0_CLK",
          "freq_mhz": 720
        },
        {
          "clk": "DEV_GPU0_GPU_PLL_CLK",
          "freq_mhz": 720
        }
      ],
      [
        {
          "clk": "CLK_J722S_GLUELOGIC_HFOSC0_CLKOUT",
          "freq_mhz": 25
        },
        {
          "clk": "CLK_J722S_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK",
          "freq_mhz": 2000
        },
        {
          "clk": "CLK_J722S_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK",
          "freq_mhz": 500
        },
        {
          "clk": "CLK_J722S_SAM62_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK",
          "freq_mhz": 500
        },
        {
          "clk": "CLK_J722S_SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK",
          "freq_mhz": 500
        },
        {
          "clk": "DEV_GPU0_PLL_CTRL_CLK",
          "freq_mhz": 500
        }
      ]
    ],

```

#### Junction Temperature

This JSON file contains junction temperature data for the SoC. Each object in the list represents the temperature measured by a sensor located at the specified core and vddDomain.

- ``tempMiliDegC`` - The temperature in millidegrees Celsius.
- ``description`` - A brief description of the location of the sensor in the SoC.

### Console output

Print levels determine the verbosity of the output.

- `0` - Minimal output. Only the most essential information is printed (default).
- `1` - Intermediate output. Some additional information is printed.
- `2` - Full output. All available information is printed.

Below are the trimmed down prints available at different print levels.

#### PLL

##### Print Level  0

```
┌────────────────────────────────────────────┐
│          PLL Configuration Table           │
├────────────┬────────────┬──────────────────┤
│ Name       │ PLL Status │ VCO Freq(MHz)    │
├────────────┼────────────┼──────────────────┤
│ MAIN_0     │ ENABLED    │ 2000             │
├────────────┼────────────┼──────────────────┤
│ MAIN_1     │ ENABLED    │ 1920             │
├────────────┼────────────┼──────────────────┤
 ...
```

##### Print Level  1

```
┌───────────────────────────────────────────────────────────────────────────────────────┐
│                                PLL Configuration Table                                │
├────────────┬────────────┬──────────────────┬──────────┬────────────┬──────────────────┤
│ Name       │ PLL Status │ VCO Freq(MHz)    │ HSDIV    │ Status     │ Output Freq(MHz) │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│ MAIN_0     │ ENABLED    │ 2000             │ HSDIV0   │ ENABLED    │ 500              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV1   │ ENABLED    │ 200              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV2   │ ENABLED    │ 200              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV3   │ ENABLED    │ 133.333333       │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV4   │ ENABLED    │ 80               │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV5   │ ENABLED    │ 200              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV6   │ ENABLED    │ 200              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV7   │ ENABLED    │ 200              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV8   │ ENABLED    │ 50               │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV9   │ ENABLED    │ 100              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│ MAIN_1     │ ENABLED    │ 1920             │ HSDIV0   │ ENABLED    │ 192              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤
│            │            │                  │ HSDIV1   │ ENABLED    │ 160              │
├────────────┼────────────┼──────────────────┼──────────┼────────────┼──────────────────┤

...
```

##### Print Level  2

```
┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                            PLL Configuration Table                                                             │
├────────────┬────────────┬────────────────────────┬──────────────────┬───────────────┬───────────────┬──────────┬────────────┬──────────────────┤
│ Name       │ PLL Status │ Config                 │ PostDiv          │ VCO Freq(MHz) │ Num of HSDIVs │ HSDIV    │ Status     │ Output Freq(MHz) │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│ MAIN_0     │ ENABLED    │ M=80, FracM=0          │ ENABLED, M2=2    │ 2000          │ 10            │ HSDIV0   │ ENABLED    │ 500              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV1   │ ENABLED    │ 200              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV2   │ ENABLED    │ 200              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV3   │ ENABLED    │ 133.333333       │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV4   │ ENABLED    │ 80               │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV5   │ ENABLED    │ 200              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV6   │ ENABLED    │ 200              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV7   │ ENABLED    │ 200              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV8   │ ENABLED    │ 50               │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV9   │ ENABLED    │ 100              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│ MAIN_1     │ ENABLED    │ M=76, FracM=13421773   │ ENABLED, M2=2    │ 1920          │ 7             │ HSDIV0   │ ENABLED    │ 192              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV1   │ ENABLED    │ 160              │
├────────────┼────────────┼────────────────────────┼──────────────────┼───────────────┼───────────────┼──────────┼────────────┼──────────────────┤
│            │            │                        │                  │               │               │ HSDIV2   │ ENABLED    │ 192              │
```

#### PSC

##### Print Level  0

```
┌──────────────────────────────────────────────┐
│           PSC Configuration Table            │
├──────────────┬──────────────────┬────────────┤
│ PSC Name     │ PD Name          │ PD Status  │
├──────────────┼──────────────────┼────────────┤
│ WKUP_PSC0    │ GP_core_CTL_MCU  │ ENABLED    │
├──────────────┼──────────────────┼────────────┤
│              │ PD_MCUSS         │ ENABLED    │
├──────────────┼──────────────────┼────────────┤
│ PSC0         │ GP_CORE          │ ENABLED    │
├──────────────┼──────────────────┼────────────┤
│              │ PD_GPU_CORE      │ DISABLED   │
├──────────────┼──────────────────┼────────────┤
│              │ PD_CPSW          │ DISABLED   │
├──────────────┼──────────────────┼────────────┤
│              │ PD_MPU_CLST0     │ DISABLED   │
├──────────────┼──────────────────┼────────────┤
│              │ PD_MPU_CLST0_COR │ DISABLED   │
│              │ E0               │            │
├──────────────┼──────────────────┼────────────┤
```

##### Print Level  1

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                               PSC Configuration Table                               │
├──────────────┬──────────────────┬────────────┬────────────────────────┬─────────────┤
│ PSC Name     │ PD Name          │ PD Status  │ LPSC Name              │ LPSC Status │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│ WKUP_PSC0    │ GP_core_CTL_MCU  │ ENABLED    │ LPSC_mcu_alwayson      │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_main2mcu_ISO      │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_DM2MCU_ISO        │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_DM2safe_ISO       │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_mcu2DM_ISO        │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_mcu_test          │ DISABLED    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │ PD_MCUSS         │ ENABLED    │ LPSC_mcu_r5            │ DISABLED    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_MCU_mcanss_0      │ DISABLED    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_MCU_mcanss_1      │ DISABLED    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_MCU_common        │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_MCU_PBIST         │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│ PSC0         │ GP_CORE          │ ENABLED    │ LPSC_main_alwayson     │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_main_dm           │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_main_dm_pbist0    │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
│              │                  │            │ LPSC_main_main2dm_iso  │ ENABLED     │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┤
```

##### Print Level  2

```
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                 PSC Configuration Table                                                  │
├──────────────┬──────────────────┬────────────┬────────────────────────┬─────────────┬────────────────────────────────────┤
│ PSC Name     │ PD Name          │ PD Status  │ LPSC Name              │ LPSC Status │ Controlled IP Instances            │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│ WKUP_PSC0    │ GP_core_CTL_MCU  │ ENABLED    │ LPSC_mcu_alwayson      │ ENABLED     │ MCU_CTRL_MMR0                      │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ MCU_DCC0                           │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ MCU_DCC1                           │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ MCU_GPIO0                          │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ MCU_MCU_SEC_MMR0                   │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ MCU_PADCFG_CTRL0                   │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ WKUP_ESM0                          │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ WKUP_MCU_GPIOMUX_INTROUTER0        │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ WKUP_PLL0                          │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │ LPSC_main2mcu_ISO      │ ENABLED     │                                    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │ LPSC_DM2MCU_ISO        │ ENABLED     │                                    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │ LPSC_DM2safe_ISO       │ ENABLED     │                                    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │ LPSC_mcu2DM_ISO        │ ENABLED     │                                    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │ LPSC_mcu_test          │ DISABLED    │                                    │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │ PD_MCUSS         │ ENABLED    │ LPSC_mcu_r5            │ DISABLED    │ MCU_R5FSS0_CORE0                   │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
│              │                  │            │                        │             │ MCU_RTI0                           │
├──────────────┼──────────────────┼────────────┼────────────────────────┼─────────────┼────────────────────────────────────┤
```

#### PET

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                            Frequency of Selected IPs                            │
├──────────────────────────────┬────────────┬────────────────────────┬────────────┤
│ IP Name                      │ Freq(MHz)  │ LPSC                   │ Status     │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN SMS 0                   │ 400        │ LPSC_main_alwayson     │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ Domain Manager(WKUP) R5FSS 0 │ 800        │ LPSC_main_dm           │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN R5FSS 1                 │ 800        │ LPSC_main_mcuss0_core0 │ DISABLED   │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MCU R5FSS 0                  │ 800        │ LPSC_mcu_r5            │ DISABLED   │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN A53SS 0                 │ 1250       │ LPSC_main_mpu_clst0_co │ DISABLED   │
│                              │            │ re0                    │            │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN C71SS 0                 │ 1000       │ LPSC_main_c7dsp0_core  │ DISABLED   │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN C71SS 1                 │ 1000       │ LPSC_main_c7dsp1_commo │ DISABLED   │
│                              │            │ n                      │            │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN VPAC 0                  │ 600        │ LPSC_main_vpac         │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN DMPAC 0                 │ 428.571428 │ LPSC_pdrsvd0_rsvd0     │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN GPU 0                   │ 720        │ LPSC_main_gpu_ctrl     │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN Video Encoder/Decoder 0 │ 500        │ LPSC_main_codec        │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN JPEG Encoder 0          │ 250        │ LPSC_main_jpeg         │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ LPDDR4 EMIF 0                │ 933.25     │ LPSC_main_emif_local   │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MCU SS /WKUP Module CLK (MCU │ 400        │ LPSC_mcu_alwayson      │ ENABLED    │
│  SYSCLK)                     │            │                        │            │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ MAIN Module CLK (SYSCLK)     │ 500        │ LPSC_main_alwayson     │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ Device Manager Domain CLK    │ 400        │ LPSC_mcu_alwayson      │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ HSM Domain CLK               │ 500        │ LPSC_main_hsm          │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ CSI TX ESC CLK               │ 16         │ LPSC_main_csi_tx0      │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ CSI TX MAIN CLK              │ 500        │ LPSC_main_csi_tx0      │ ENABLED    │
└──────────────────────────────┴────────────┴────────────────────────┴────────────┘
```

#### Clock Tree

```
Device IP: J722S_DEV_GPU0
Number of clocks: 2

  ▶  GLUELOGIC_HFOSC0_CLKOUT (25 MHz) --> PLLFRACF2_SSMOD_16FFT_MAIN_6_FOUTVCOP_CLK (2160 MHz) --> HSDIV0_16FFT_MAIN_6_HSDIVOUT0_CLK (720 MHz) --> GPU_PLL_CLK (720 MHz)
  ▶  GLUELOGIC_HFOSC0_CLKOUT (25 MHz) --> PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK (2000 MHz) --> HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK (500 MHz) --> SAM62_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK (500 MHz) --> SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK (500 MHz) --> PLL_CTRL_CLK (500 MHz)
─────────────────────────────────────────────────────────────────────────────────────────

```

NOTE: Some clocks of IPs may originate from sources other than the primary HFOSC/RC, in which case the clock tree print will not display the partial path and instead say "Refer to clock architecture."

#### Junction Temperature

```
Sensor  Location        Temperature (°C)
0       Near DSS OLDI, in-between two C7x       46.12
1       In-between GPU, VPAC and A53            46.34
2       Near A53 and LPDDR                      47.25
```

## User Config

To print power estimation details of specific IPs or modules, add configuration in `config/.meta/<soc>/custom_ip_mapping.json`.
Running the tool with the `--selector pet` argument will then print power estimation details of the specified IP or module.

The configuration in `custom_ip_mapping.json` should be in the following format:

```
{
    "CSI TX ESC CLK": {
		"ip_name": "J722S_DEV_CSI_TX_IF0",
		"input_name": "DEV_CSI_TX_IF0_ESC_CLK_CLK",
		"tisci_ip_name": "TISCI_DEV_CSI_TX_IF0",
		"tisci_input_name": "TISCI_DEV_CSI_TX_IF0_ESC_CLK_CLK",
		"lpsc_name": "LPSC_main_csi_tx0",
		"programmable": true,
		"default_freq_mhz": 16,
        "enableLpsc": true
	},
}
```

`ip_name` is the IP name, `input_name` is the particular clock of that IP, and `lpsc_name` is the LPSC of that IP.
Refer to [TISCI](https://software-dl.ti.com/tisci/esd/latest/5_soc_doc) for IP name & TRM for LPSC name.

Refer to \subpage CUSTOM_CONFIG documentation.

Output

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                            Frequency of Selected IPs                            │
├──────────────────────────────┬────────────┬────────────────────────┬────────────┤
│ IP Name                      │ Freq(MHz)  │ LPSC                   │ Status     │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ CSI TX ESC CLK               │ 16         │ LPSC_main_csi_tx0      │ ENABLED    │
├──────────────────────────────┼────────────┼────────────────────────┼────────────┤
│ CSI TX MAIN CLK              │ 500        │ LPSC_main_csi_tx0      │ ENABLED    │
└──────────────────────────────┴────────────┴────────────────────────┴────────────┘

```

## Supported SoCs {#supported-socs}

The tool supports the following SoCs:

- j784s4
- j721s2
- j721e
- j722s
- j7200

## Supported Debuggers {#supported-debuggers}

- CCS20.xx and above or CCS12.xx and above
- Trace32 PowerView for ARM version N.2025.01.xx or above with openOCD 0.12.0 or above

##  Known Issues

- None

## Troubleshoot

This section details troubleshooting techniques for resolving issues that may arise when using the JTAG Power Analysis Tool.

1. **Missing Memory Data Initialization**

     ```

      File /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/memory_values_csv/j722s does not exist

      Memory Data is not initialized

     ```

     **Solution:** Ensure you've successfully completed Step 1 of the tool's setup instructions. This involves creating the MMR key-value pair data and providing the correct path to the tool.

2. **MMR Read Failed - Incorrect Selector**

     ```

      Memory not found 67109376 0x4000200

      /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/src/drv/lpsc.js:21

              throw new Error("Fatal error: MMR read failed. MMR key value data is missing.");

              ^

      Error: Fatal error: MMR read failed. MMR key value data is missing.

    ```

     **Explanation:** This error occurs when running the tool with a different `selector` than the one used during the debugger
     execution.  For example, running the debugger with `selector="pll"` and the tool with `selector="pet/psc"`.  The debugger needs to
     retrieve the required MMRs to generate outputs accurately.

     **Solution:**   Use `selector="all"` in `pmDataGenerator(soc="j722s",selector = "all")` to ensure all necessary MMR data is
     collected.

3. **Missing Node Modules**

     ```

      Error: Cannot find module 'minimist'

     ```

     **Solution:** Navigate to the `jtag_power_analysis_tool/` directory and run either `npm ci` or `npm install` to install the required Node.js modules.

4. **Script Path Error**

     ```

      js:> pmDataProcessor(soc="j722s",selector="all")

      Error: pm_data_generator_ccs20: file /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/debugger_scripts/ccs20/pm_data_generator_ccs20.js not found, please check the path in script

     ```

     **Solution:**  Update the `filePath` variable within the script to the *absolute* path of the `pm_data_generator_ccs20.js` file.  This will ensure the tool can locate and execute the necessary script.
