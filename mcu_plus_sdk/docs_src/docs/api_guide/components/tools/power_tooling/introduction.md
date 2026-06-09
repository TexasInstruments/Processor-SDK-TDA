
## Introduction {#INTRODUCTION}

Power tooling enables users to configure power settings, such as PLL and PSC, tailored to their specific use cases. Users can use power tools in tandem to achieve this. The JTAG power analysis tool is used to analyze the power settings. This information is then used to refine the configuration via the DM power config tool. Users can significantly reduce power consumption by selecting which IPs and modules to activate or deactivate and adjusting their frequencies.
For setups without a JTAG interface, the DM power analysis tool serves as an alternative to the JTAG power analysis tool, providing the same power settings information.

\imageStyle{power_analysis_and_config_tools.png,width:50%}
\image html power_analysis_and_config_tools.png

### JTAG Power Analysis Tool

This tool is used to get the power settings of the SoC at any instance using the JTAG interface. Users can get the IP frequencies, PLL settings, PSC status, and clock tree of the SoC.
Refer \subpage JTAG_POWER_ANALYSIS_USERGUIDE

### DM Power Configuration Tool

This tool provides a GUI to configure the IP frequencies and turn the PSC of various modules on/off.
Refer \subpage DM_POWER_CONFIG_USERGUIDE

### DM Power Analysis Tool

The DM Power Analysis Tool is used to obtain the PLL and divider settings, PSC status, and clock tree of the SoC.
It is a C-based tool that doesn't require a JTAG connection. It runs on one of the cores in the device. The PLL and divider configuration is obtained by reading the control registers, and the clock tree is obtained using the Power Management Clock API.
Refer \subpage DM_POWER_ANALYSIS_TOOL

## SoC Compatibility Matrix

### Tool Support by SoC

| Tool                     | J784S4 | J722S | J721S2 | J721E | J7200 |
| ------------------------ | :----: | :---: | :----: | :---: | :---: |
| DM Power Config Tool     |   ✓    |   ✓   |   ✓    |   ✗   |   ✗   |
| JTAG Power Analysis Tool |   ✓    |   ✓   |   ✓    |   ✓   |   ✓   |
| DM Power Analysis Tool   |   ✓    |   ✓   |   ✓    |   ✓   |   ✓   |

### JTAG Power Analysis Tool Features by SoC

| Feature    | J784S4 | J722S | J721S2 | J721E | J7200 |
| ---------- | :----: | :---: | :----: | :---: | :---: |
| PET        |   ✓    |   ✓   |   ✓    |   ✗   |   ✗   |
| PLL        |   ✓    |   ✓   |   ✓    |   ✓   |   ✓   |
| PSC        |   ✓    |   ✓   |   ✓    |   ✓   |   ✓   |
| CLOCK-TREE |   ✓    |   ✓   |   ✓    |   ✓   |   ✓   |

### JTAG Power Analysis Tool Features by Debuggers

| Feature              | CCS20  | CCS12 | Trace32|
| -------------------- | :----: | :---: | :----: |
| PET                  |   ✓    |   ✓   |   ✓    |
| PLL                  |   ✓    |   ✓   |   ✓    |
| PSC                  |   ✓    |   ✓   |   ✓    |
| CLOCK-TREE           |   ✓    |   ✓   |   ✓    |
| JUNCTION TEMPERATURE |   ✓    |   ✗   |   ✗    |

✓ = Supported
✗ = Not Supported
