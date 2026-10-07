# CLK – Periodic Software Read-back of Static Configuration Registers {#IP_FMA_CLK_PAGE}

[TOC]

# Overview

This safety mechanism is intended to periodically monitor and verify the configuration
and operation of the Dual Clock Comparator (DCC) module. Since the DCC monitors clock sources
and detects frequency drifts, maintaining the integrity of its configuration registers
is essential for ensuring correct clock comparison and safe operation of the system.

DCC configuration registers may be affected by software errors, misconfigurations,
or unexpected hardware behavior. Therefore, periodic software readback is required
to verify that the DCC static configuration has not changed unexpectedly.

This safety mechanism (SM) is implemented as a part of the IP FMA structure as it is intended
to be a safety diagnostics test for the Dual Clock Comparator (DCC) module. It
is identified as `CLK` in the TI Functional Safety Manual as the *Periodic
Software Read Back of Static Configuration Registers* safety mechanism.

A set of static DCC registers is periodically read back to verify that the module's
configuration remains consistent following DCC configuration and operation.
After configuring the reference and test clocks, the DCC is enabled,
and the register values are periodically read back to monitor the system
and verify that no unexpected changes have occurred.

This safety mechanism is a part of the IP FMA structure
along with its baremetal example application. This application configures the DCC module for comparison of two clock sources
in single-shot mode, enables interrupts for normal completion, and performs periodic readback of DCC configuration registers
to ensure correct operation.
It can be found in the
`<mcu_plus_sdk>/examples/ip_fma/clk_ip_fma_app` directory.

This SM code is a part of the MCU+ SDK repository.
Read and write operations are done via IP FMA library functions which source
code can be found inside `<mcu_plus_sdk>/source/ip_fma/src`. IP FMA library
implements DCC Register check and defines static configuration register offsets.

# Preconditions and Assumptions

Here are some preconditions and assumptions for the correct usage of the SM:

* Use the following TI hardware platform: `J722S`.
* Ensure the SoC's DCC clock sources are correctly configured for the MCU_DCC0
  instance - the reference clock (HFOSC0, CLOCK0[0]) and test clock (SYSCLK0,
  dedicated CLOCK1 input).

# Build Steps

To demonstrate the usage of the safety mechanism you need to build an example
application at the path
`<mcu_plus_sdk>/examples/ip_fma/clk_ip_fma_app`.

**Step 1 - Build the example application:**

The `ip_fma` library sources are compiled directly as part of the example.
You can build this application for only the `mcu-r5fss0-0` core.

\code
cd <mcu_plus_sdk>
make -C examples/ip_fma/clk_ip_fma_app/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/
\endcode

**Step 2 - Verify the build.**
Check for `clk_ip_fma_app.release.appimage.hs_fs` in
`examples/ip_fma/clk_ip_fma_app/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/`.

**Step 3 - Flash and run via UART.**
Edit `tools/boot/sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg` to point
at your built appimage:

\code
--file=../../examples/ip_fma/clk_ip_fma_app/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/clk_ip_fma_app.release.appimage.hs_fs
\endcode

Then flash the board over UART:

\code
cd <mcu_plus_sdk>/tools/boot
python3 uart_bootloader.py -p /dev/ttyUSB2 --cfg=sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg
\endcode

# Execution Flow

After the build, load and run the application via UART boot as described
in Step 4 above. The result of building this example application is the
`clk_ip_fma_app.release.appimage.hs_fs` image, flashed onto the board using
`uart_bootloader.py`.

Also bear in mind that this code is prepared and compiled for the
`mcu-r5fss0-0` core, so the application runs on this core which means that
application logs are expected on the configured debug UART interface.

After the application is executed you will get log messages like this:

\code
DCC Config Registers Periodic Check: Start
DCC Config Registers Periodic Check: Seed values calculation done.
DCC Config Registers Periodic Check: DCC configured
DCC Config Registers Periodic Check: Enabling DCC and waiting for completion interrupt
DCC Config Registers Periodic Check: Register check starts...
DCC Config Registers Periodic Check: Loading expected DCC register values...
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
DCC Config Registers Periodic Check: DCC Generated completion interrupt
DCC Config Registers Periodic Check: No Clock Drift was observed
DCC Config Registers Periodic Check: All tests have passed.
DCC Config Registers Periodic Check: Completes!!!
\endcode

This log demonstrates a positive use case where the application starts successfully and determines the clock frequencies
for the reference and test sources, calculates the seed values required for successful DCC completion,
and configures the interrupts for normal operation. The DCC instance is then configured and enabled, and
the system waits for the completion interrupt. Periodic software readback of the DCC registers confirms that
all expected and actual register values match, indicating no clock drift. As a result, the DCC generates the completion interrupt,
all tests pass, and the system operation completes successfully without errors.

The negative test scenario verifies that the comparison mechanism correctly detects a mismatch. It is explicitly
triggered by uncommenting the `memset()` line, which clears the expected register structure and forces all expected
values to `0x00000000`. Since the hardware registers contain correctly initialized values after DCC configuration,
comparing them against this artificially-zeroed expected structure produces a mismatch, confirming that the
register-check mechanism correctly flags a deviation when one is present.

```c
/* Step 1 Determine the clock frequencies for the sources */
status = ClkApp_GetClkfreqKHz(APP_DCC_MODULE_INST, APP_DCC_REF_CLOCK_SRC_0, APP_DCC_TEST_CLOCK_SRC_1, &clk0Freq, &clk1Freq);

/* Step 2 Figure out the seed values for successful completion */
ClkApp_GetClkRatio(clk0Freq, clk1Freq, &refClkRatioNum, &testClkRatioNum);
configParams.mode    = APP_DCC_MODE;
configParams.clk0Src = APP_DCC_REF_CLOCK_SRC_0;
configParams.clk1Src = APP_DCC_TEST_CLOCK_SRC_1;
/* Get the seed values for given clock selections and allowed drift */
ClkApp_SetSeedVals(clk0Freq, clk1Freq, refClkRatioNum, testClkRatioNum, APP_DCC_TEST_CLOCK_SRC_1_DRIFT, &configParams);

/* Step 3 Configure DCC instance */
status = SDL_DCC_configure(APP_DCC_MODULE_INST, &configParams);

/* Step 4 Register ISR and configure interrupts for normal completion */
status = ClkApp_RegisterIsr(APP_DCC_MODULE_INST);

/* Get expected values from DCC registers for MCU_DCC0 instance */
IpFma_DccRegs dccRegsExpValues;
IpFma_Status statusRb = IPFMA_OK;
statusRb = IpFma_Clk_GetDccRegs(APP_DCC_MODULE_INST, &dccRegsExpValues);
/* Uncommenting this line forces the negative test case:
* all expected register values are set to 0x00000000,
* causing mismatch vs actual hardware values.
*/
memset(&dccRegsExpValues, 0, sizeof(dccRegsExpValues));
Clk_AppPrint(APP_DCC_STR ": Loading expected DCC register values...\r\n");

/* Step 5 Enable DCC */
SDL_DCC_enable(APP_DCC_MODULE_INST);

/* Step 6 Wait for completion interrupt or error flag, performing
   periodic register readback/compare on every pass while waiting */

/* Read the actual values from registers */
IpFma_DccRegs dccRegsActualValues;
statusRb = IpFma_Clk_GetDccRegs(APP_DCC_MODULE_INST, &dccRegsActualValues);

/* Compare expected and actual values periodically */
statusRb = IpFma_Clk_CompareDccRegs(&dccRegsExpValues, &dccRegsActualValues);

if (IPFMA_OK == statusRb)
{
   Clk_AppPrint(APP_DCC_STR ": values MATCH!\r\n");
}
else
{
   Clk_AppPrint(APP_DCC_STR ": values MISMATCH!\r\n");
}
```

The following log illustrates the negative test case execution:

\code
DCC Config Registers Periodic Check: Start
DCC Config Registers Periodic Check: Seed values calculation done.
DCC Config Registers Periodic Check: DCC configured
DCC Config Registers Periodic Check: Enabling DCC and waiting for completion interrupt
DCC Config Registers Periodic Check: Register check starts...
DCC Config Registers Periodic Check: Loading expected DCC register values...
DCC Config Registers Periodic Check: DONE!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MISMATCH!
DCC Config Registers Periodic Check: DCC Generated completion interrupt
DCC Config Registers Periodic Check: No Clock Drift was observed
DCC Config Registers Periodic Check: DCC register check failed!!
DCC Config Registers Periodic Check: Completes!!!
\endcode

# Limitations and Notes

No known limitations.