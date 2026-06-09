# LPM UART Wakeup {#EXAMPLES_LPM_MCU_UART_WAKEUP}

[TOC]

# Introduction

This example shows usage of RP Message APIs to exchange messages between linux on cortex-A CPU and
RTOS/NORTOS CPUs. This example also demonstrates the capability of MCU UART input event to wake the
entire SOC in MCU Only low power mode.

\attention Low power mode is supported only on the Linux SPL boot flow. SBL bootflow does not support low power mode (LPM)

In this example,
- We first confirm the functionality of IPC Message exchange between Linux and other cores. Refer \ref EXAMPLES_DRIVERS_IPC_RPMESSAGE_LINUX_ECHO. This is necessary to ensure that IPC is working correctly, which is a requirement for entering low power mode.
\cond SOC_AM62X
- This example provides support for graceful shutdown of the remote core (MCU M4F). Refer \ref GRACEFUL_REMOTECORE_SHUTDOWN
- This example provides support for MCU only low power mode support on the MCU core (MCU M4F). To run MCU-only low power mode, refer \ref EXAMPLES_LPM_MCU_UART_WAKEUP_MCU_ONLY_LPM
- This example provides support for Deep Sleep low power mode (MCU M4F). To run MCU-only low power mode, refer \ref EXAMPLES_LPM_MCU_UART_WAKEUP_DEEP_SLEEP_LPM
\endcond
\cond SOC_AM62AX
- This example provides support for graceful shutdown of the remote core (MCU R5F). Refer \ref GRACEFUL_REMOTECORE_SHUTDOWN
- This example provides support for MCU only low power mode support on the MCU core (MCU R5F). To run MCU-only low power mode, refer \ref EXAMPLES_LPM_MCU_UART_WAKEUP_MCU_ONLY_LPM
- This example provides support for Deep Sleep and IO Only plus DDR low power mode (MCU R5F/ C7X). To run MCU-only low power mode, refer \ref EXAMPLES_LPM_MCU_UART_WAKEUP_DEEP_SLEEP_LPM
\endcond
\cond SOC_AM62PX
- This example provides support for graceful shutdown of the remote core (MCU R5F). Refer \ref GRACEFUL_REMOTECORE_SHUTDOWN
- This example provides support for MCU only low power mode support on the MCU core (MCU R5F). To run MCU-only low power mode, refer \ref EXAMPLES_LPM_MCU_UART_WAKEUP_MCU_ONLY_LPM
- This example provides support for other low power modes on the remote core (MCU R5F). To run MCU-only low power mode, refer \ref EXAMPLES_LPM_MCU_UART_WAKEUP_DEEP_SLEEP_LPM
\endcond
The example integrates bootloading functionality with SBL on OSPI bootmedia. It
also integrates Device manager functionality. The SBL stage 2 thread boots all
the cores along with HLOS like Linux. Refer \ref SBL_BOOTING_LINUX_OSPI for boot
flow sequence.

# Supported Combinations

\cond SOC_AM62X

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | m4fss0-0 freertos
 Toolchain      | ti-arm-clang
 Board          | @VAR_BOARD_NAME_LOWER, @VAR_SK_LP_BOARD_NAME_LOWER, @VAR_SIP_SK_BOARD_NAME_LOWER
 Example folder | examples/lpm/lpm_mcu_dmtimer_wakeup

\endcond

\cond SOC_AM62AX

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | mcu-r5fss0-0 freertos
 Toolchain      | ti-arm-clang
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | examples/lpm/lpm_mcu_dmtimer_wakeup

\endcond

\cond SOC_AM62PX

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | mcu-r5fss0-0 freertos
 Toolchain      | ti-arm-clang
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | examples/lpm/lpm_mcu_dmtimer_wakeup

\endcond
# Steps to Run the Example

- **When using CCS projects to build**, import the system CCS project
  and build it using the CCS project menu (see \ref CCS_PROJECTS_PAGE). This will build all the dependant CPU projects as well
- **When using makefiles to build**, build the system makefile using
  make command (see \ref MAKEFILE_BUILD_PAGE). This will build all the dependant CPU makefiles as well.
- To run this demo, Linux needs to run on the Cortex A-core. Refer to **Processor SDK Linux** user guide to load and run this example.

- This example integrates SBL on OSPI bootmedia which needs to be flashed on the
EVM flash, along with sample application images for MCU R5 CPUs, HSM M4F and
Linux Appimage.

- For HS-FS device, use **default_sbl_ospi_linux_hs_fs.cfg** as the cfg file.

- To flash to the EVM, refer to \ref GETTING_STARTED_FLASH .

- Example, assuming SDK is installed at `C:/ti/mcu_plus_sdk` and this example
and IPC application is built using makefiles, and Linux Appimage is already
created, in Windows,

        cd C:/ti/mcu_plus_sdk/tools/boot
        python uart_uniflash.py -p COM13 --cfg=C:/ti/mcu_plus_sdk/tools/boot/sbl_prebuilt/@VAR_BOARD_NAME_LOWER/default_sbl_ospi_linux_hs_fs.cfg

- If Linux PC is used, assuming SDK is installed at `~/ti/mcu_plus_sdk`

        cd ~/ti/mcu_plus_sdk
        python uart_uniflash.py -p /dev/ttyUSB0 --cfg=~/ti/mcu_plus_sdk/tools/boot/sbl_prebuilt/@VAR_BOARD_NAME_LOWER/default_sbl_ospi_linux_hs_fs.cfg

- Switch to \ref BOOTMODE_OSPI and power on the EVM.

\attention As the wake-up R5 is the device manager, it needs to be started by
the SBL. So it can not be loaded through CCS. It should be flashed and booted
through SBL.

## MCU only LPM {#EXAMPLES_LPM_MCU_UART_WAKEUP_MCU_ONLY_LPM}

- Set the wake up resume latency as 100ms for CPU0 on the linux kernel by running the following command. When the resume latency value is less, suspending the kernel will go to `MCU only sleep mode`.

\code
$ echo 100000 > /sys/devices/system/cpu/cpu0/power/pm_qos_resume_latency_us
\endcode

- Go to MCU only low power mode by running the following command on the linux.

\code
$ echo mem > /sys/power/state
\endcode

- After this the following message will appear on the MCU UART.

\code
[LPM UART WAKEUP] Next MCU mode is 1
[LPM UART WAKEUP] Suspend request to MCU-only mode received
[LPM UART WAKEUP] Press a single key on this terminal to resume the kernel from MCU only mode
\endcode

- Then type any key on the MCU UART to resume the kernel from LPM.

\code
[LPM UART WAKEUP] Key pressed. Notifying DM to wakeup main domain
[LPM UART WAKEUP] Main domain resumed due to MCU UART
\endcode

- For detailed implementation of low power modes, refer \ref LOW_POWER_MODE_AWARE_REMOTECORE

## Deep Sleep LPM {#EXAMPLES_LPM_MCU_UART_WAKEUP_DEEP_SLEEP_LPM}

- Go to Deep Sleep low power mode by running the following command on the linux.

\code
$ echo mem > /sys/power/state
\endcode

- After this, the following message will appear on the MCU UART.

\code
[LPM UART WAKEUP] Next MCU mode is 0
[LPM UART WAKEUP] Closing all drivers and going to WFI ... !!!
\endcode

- Then to resume the kernel from LPM, press any key on MCU UART. This will trigger the IO Daisy chain wakeup, which will resume the kernel. The following message will appear on the MCU UART

\code
[LPM UART WAKEUP] Example Application Started...
[LPM UART WAKEUP] Remote Core waiting for messages at end point 13 ... !!!
[LPM UART WAKEUP] Remote Core waiting for messages at end point 14 ... !!!

\endcode

# See Also

- \ref DRIVERS_IPC_RPMESSAGE_PAGE
- \ref EXAMPLES_LPM_MCU_MCAN_WAKEUP
- \ref EXAMPLES_LPM_MCU_DMTIMER_WAKEUP

# Sample Output

There is no direct output from the RTOS/NORTOS CPUs on the UART or CCS console.
The output is seen on the Linux console on Cortex-A CPU.

