# CSI RX – Periodic Software Readback of Static Configuration Registers {#IP_FMA_CSIRX_PAGE}

[TOC]

## Overview

The CSI RX (Camera Serial Interface Receiver) module on TI SoCs is responsible
for receiving image and video data from external camera sensors over one or
more MIPI CSI2 lanes. The module performs lane synchronization, packet
decoding, virtual-channel routing, and places the received pixel data into
memory through the CSI RX DMA subsystem.

The safety mechanism is intended to **periodically read back** all configuration
written to the CSI RX hardware registers after initialization of the module.
Software readback is essential for verifying that the applied configuration
matches the expected settings, especially in systems where register writes may
be indirectly modified due to resets, low-power transitions, or other dynamic
changes. Periodic readback of configuration registers can provide a diagnostic
for inadvertent writes or disturbance of these registers.

The software readback mechanism provides visibility into all programmed CSI RX
control registers, assists in debugging pipeline bring-up issues, and ensures
proper configuration validation in safety-related systems. It does not modify
any hardware state; it only retrieves and checks the configuration.

This safety mechanism is a part of the IP FMA structure
with its bare-metal example application. This application performs a simple
readback of static configuration registers and can be found inside the
`$(MCU_PLUS_SDK_PATH)/examples/ip_fma/csirx_static_regs_readback`
directory.

The example application
(`$(MCU_PLUS_SDK_PATH)/examples/ip_fma/csirx_static_regs_readback`) is a
simple bare-metal application which enables the CSI RX module clocks, captures
the initial values of selected static registers as the expected values, and
periodically compares them with newly read values. Register readback and
comparison are done via **IP FMA library** functions, whose
source code can be found inside
`$(MCU_PLUS_SDK_PATH)/source/ip_fma/src`.
The **IP FMA library** implements the CSI RX register check and defines static
configuration register offsets.

## Preconditions and Assumptions

Here are some preconditions and assumptions for correct usage of the safety
mechanism:

* Use the TI `J722S-EVM` hardware platform.
* Make sure that the J722S MCU+ SDK build environment and TI Arm Clang
  toolchain are configured correctly. Required MCU+ SDK libraries and boot
  components should be built before running the example.
* When executing the application through the SBL UART flashing flow, prepare
  the J722S-EVM for UART boot mode. Refer to the J722S MCU+ SDK EVM Setup and
  flashing documentation for boot-mode configuration and UART flashing
  instructions.

## Build Steps

To demonstrate the usage of the safety mechanism you need to build the example
application which enables the CSI RX modules and reads back selected static
registers using the safety mechanism code.

At the path
`$(MCU_PLUS_SDK_PATH)/examples/ip_fma/csirx_static_regs_readback/`
there is an example application which enables the CSI RX module clocks and
performs periodic readback. The application uses the initial register values
as the expected baseline for subsequent comparisons.

1. First build this example:

    \code
    cd $(MCU_PLUS_SDK_PATH)
    make -f makefile.j722s ip_fma_csirx_j722s-evm_mcu-r5fss0-0_nortos_ti-arm-clang PROFILE=release
    \endcode

    Note that this code is prepared and compiled for the `mcu-r5fss0-0`
    core using the TI Arm Clang toolchain. The application runs on the MCU R5F
    core and its log output is available through the UART interface configured
    for the example.

2. Verify that the build process completed successfully. The build should
   produce the application executable, such as
   `ip_fma_csirx.release.out`, in the target build output.

   When the application is prepared for an SBL boot flow, the corresponding
   J722S application image is generated according to the selected MCU+ SDK
   device and boot configuration.

3. Load or flash the generated application on the J722S-EVM using the selected
   MCU+ SDK execution method.

## Execution Flow

After the build, what remains is to run the program on the J722S-EVM.
The application can be loaded using the supported MCU+ SDK execution flow.
When using an SBL application image, follow the J722S MCU+ SDK UART flashing
procedure and configure the board for the required boot mode.

The following sequence describes how the periodic readback task operates:

* The application initiates a readback callback.
* The callback reads all CSI RX registers.
* The register values are stored into a memory-resident structure.
* The previously read register values are compared with expected values.
* A match/mismatch of the compared values is reported.
* The mechanism repeats at the configured periodic interval.

## Example Readback Snippet

\code{.c}
// Get expected values from CSI RX shim registers.
IpFma_CsirxShimRegs shimExpected;
shimStatus = IpFma_Csirx_GetShimRegs(CSI_RX_SHIM_DOMAINS_BASE[csi_rx_domain], &shimExpected);

/******************************************************
               Periodic readback
******************************************************/

// Read the actual values from CSI RX shim registers.
IpFma_CsirxShimRegs shimActual;
shimStatus = IpFma_Csirx_GetShimRegs(CSI_RX_SHIM_DOMAINS_BASE[csi_rx_domain], &shimActual);

// Compare expected and actual values periodically.
shimStatus = IpFma_Csirx_CompareShimRegs(&shimExpected, &shimActual);

if (IPFMA_OK == shimStatus)
    DebugP_log(APP_CSIRX_STR ": Comparing SHIM registers... values MATCH!\r\n");
else
    DebugP_log(APP_CSIRX_STR ": Comparing SHIM registers... values MISMATCH!\r\n");
\endcode

## Output Logs

After the application is loaded and executed, log messages similar to the
following are expected:

\code
Calling CsirxApp_PeriodicReadback()

CSIRX IP FMA Example: Start
CSIRX IP FMA Example: Loading expected register values...
CSIRX IP FMA Example: Register check starts...
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: CSIRX0 register check passed
CSIRX IP FMA Example: Loading expected register values...
CSIRX IP FMA Example: Register check starts...
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: CSIRX1 register check passed
CSIRX IP FMA Example: Loading expected register values...
CSIRX IP FMA Example: Register check starts...
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: CSIRX2 register check passed
CSIRX IP FMA Example: Loading expected register values...
CSIRX IP FMA Example: Register check starts...
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: Comparing DPHY registers... values MATCH!
CSIRX IP FMA Example: Comparing SHIM registers... values MATCH!
CSIRX IP FMA Example: CSIRX3 register check passed
CSIRX IP FMA Example: All tests have passed. Completes!!!
\endcode
