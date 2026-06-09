# JTAG Power Analysis Tool with Code Composer Studio (CCS) {#JTAG_CCS}

This document provides instructions for running JTAG Power Analysis Tool with Code Composer Studio


## Requirements:
[CCS20xx platform](https://software-dl.ti.com/ccs/esd/documents/users_guide_ccs_20.1.1)
or [CCS12xx platform](https://www.ti.com/tool/download/CCSTUDIO/12.7.0)

**Before Running the Script** :

Update the `filePath` variable in the following files to the absolute path of the files:

- `tools/jtag_power_analysis_tool/debugger_scripts/ccs12/pm_data_generator_ccs12.js`
- `tools/jtag_power_analysis_tool/debugger_scripts/ccs20/pm_data_generator_ccs20.js`
- `tools/jtag_power_analysis_tool/debugger_scripts/ccs20/read_temperature_ccs20.js`
- `tools/jtag_power_analysis_tool/src/main_pm_data_processor.js`

This step is necessary for the scripts to run and locate the necessary files needed to run.

```
/* ------------------------------------- EDIT FILE PATH TO ABSOLUTE PATH BEFORE RUNNING ------------------------------------- */
filePathPm = "/home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/debugger_scripts/ccs20/pm_data_generator_ccs20.js";
```

## Running from CCS Scripting Console

The JTAG Power Analysis Tool operates in two steps:

**Step 1:**
A debugger script is executed to read the MMRs and generate a list of key-value pairs.

**Step 2:**
The tool utilizes the list of key-value pairs to generate comprehensive outputs.


### Step 1 - Generating Memory Mapped Registers (MMRs) Key-Value data
In this step, user needs to generate memory key-value pairs using CCS12/CCS20. The memory addresses that is required to read is present in `tools/jtag_power_analysis_tool/soc_data/<soc>/mem_addr`. After following the below step, key-value pair will be generated in `tools/jtag_power_analysis_tool/memory_values_csv/<soc>`.

1. Open CCS and launch target config
2. Open the Scripting console (view->console->scripting console)
3. Load JS file

   For CCS20xx:
   Load file `pm_data_generator_ccs20.js` (Load JS File using UI or type `.load <path_to_pm_data_generator_ccs20.js>`)

   For CCS12xx:
   Load file `pm_data_generator_ccs12.js` (Load JS File using UI or type ` loadJSFile <path_to_pm_data_generator_ccs12.js>`)

4. Connect to WKUP/MCU core (if not connected) and step to the point where you want to run the tool
5. Run the script API
   `pmDataGenerator(soc=<soc>,selector=<>)`

   Example
   For CC12xx

     `loadJSFile /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/debugger_scripts/ccs12/pm_data_generator_ccs12.js`

     `pmDataGenerator(soc="j722s",selector="all")`

   For CC20xx

   `.load /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/debugger_scripts/ccs20/pm_data_generator_ccs20.js`

   `pmDataGenerator(soc="j722s",selector="all")`


Sample run:
```
js:> loadJSFile /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/debugger_scripts/ccs12/pm_data_generator_ccs12.js

js:> pmDataGenerator(soc="j722s",selector="all")
File generated: /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/memory_values_csv/j722s/soc_pll_data_addr_val.csv
File generated: /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/memory_values_csv/j722s/soc_clk_data_div_addr_val.csv
File generated: /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/memory_values_csv/j722s/soc_psc_data_addr_val.csv
File generated: /home/cgt/ti/jacinto/workarea/pdk/packages/ti/drv/pdm_utils/tools/jtag_power_analysis_tool/memory_values_csv/j722s/soc_clk_data_mux_addr_val.csv
0.0

```

#### Usage Options

| Args        | Description                                                         |
| ----------- | ----------------------------------------------------------------    |
| --soc       | The SoC to be used                                                  |
| --selector  | all : runs all tools (default)                                      |
|             | pll : runs pll tool                                                 |
|             | psc : runs psc tool                                                 |
|             | pet : print PET tool IP                                             |
|             | clock_tree: runs clk_tree tool                                      |

### Step 2 - Tool Generates Output Files

#### From CCS20 Scripting Console

1. Load `main_pm_data_processor` JS File using UI or type `.load <path_to_main_pm_data_processor.js>`

2. run `pmDataProcessor(<soc>, <selector>, <level>);`

Example:

`.load /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/src/main_pm_data_processor.js`

`pmDataProcessor("j722s", "pet", 0);`


#### From Terminal
1. Run
  ` node <path to main_pm_data_processor.js> --soc <soc>  --selector <all/pll/psc/clock_tree> --level <0/1/2> [--mmrKeyValue <path to folder containing mmr key-value pair csv files>]`


The `--mmrKeyValue` option is optional and it specifies the path to the folder
containing the MMR key-value pair CSV files. If not provided, the tool will
use the default values from the
`tools/jtag_power_analysis_tool/memory_values_csv/<soc>` folder, which is
generated by the debugger script.

2. View the output in the console or saved json file in the `output` directory

Sample run:
```
node /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/src/main_pm_data_processor.js  --soc j722s --selector pet --level 0 --mmrKeyVal /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/memory_values_csv/j722s/


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
#### Usage Options

| Args          | Description                                                         |
| -----------   | ----------------------------------------------------------------    |
| --soc         | The SoC to be used                                                  |
| --selector    | all : runs all tools (default)                                      |
|               | pll : runs pll tool                                                 |
|               | psc : runs psc tool                                                 |
|               | pet : print PET tool IP                                             |
|               | clock_tree: runs clk_tree tool                                      |
| --level       | <0-2> : Print level (0 - default)                                   |
| --mmrKeyValue | Optional path to mmrKeyVal dir generated by debugger script          |

## Reading SoC Junction temperature

SoC junction temperature can be read using `read_temperature_ccs20.js` script.

Steps:
1.  Load `read_temperature_ccs20.js` JS File using UI or type `.load <path_to_read_temperature_ccs20.js>`
2.  run `getSocTemperature(<soc>)`;

Example:

```
js:> .load /home/cgt/ti/pdm_utils/tools/jtag_power_analysis_tool/debugger_scripts/ccs20/read_temperature_ccs20.js
js:> getSocTemperature("j722s");
Sensor  Location        Temperature (°C)
0       Near DSS OLDI, in-between two C7x       46.12
1       In-between GPU, VPAC and A53            46.34
2       Near A53 and LPDDR                      47.25

```



