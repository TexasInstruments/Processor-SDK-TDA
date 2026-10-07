# DRU – Periodic Software Readback of Static Configuration Registers {#IP_FMA_DRU_REGS_PERIODIC_READBACK_PAGE}

[TOC]

# Introduction

The Data Routing Unit (DRU) is a high bandwidth routing engine with programmable
DMA transfer requests. It moves data between memory mapped slave endpoints,
processor caches and shared caches, and behaves like a DMA transfer controller
running at CPU frequency.

Once the DRU has been configured, its static configuration registers are not
expected to change for as long as the module is in use. Reading them back at a
fixed interval and comparing them against the values captured after
initialisation detects a register that has been corrupted or overwritten.

The implementation is part of the IP FMA component:

* `source/ip_fma/src/ip_fma_dru.c` reads the DRU queue and channel registers and
  compares an expected register set against an actual one.
* `source/ip_fma/inc/ip_fma_dru.h` declares the API and the register structures.
* `source/ip_fma/src/ip_fma_common.c` implements the generic register readback
  used by all IP FMA mechanisms.
* `examples/ip_fma/dru_static_regs_readback` is the example application
  described on this page.

# Supported Combinations {#IP_FMA_DRU_COMBOS}

\cond SOC_J722S

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | main-r5fss0-0 freertos
 ^              | c75ss0-0 freertos
 ^              | c75ss1-0 freertos
 Toolchain      | ti-arm-clang
 ^              | ti-c7000
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | examples/ip_fma/dru_static_regs_readback

\endcond

# DRU Instances

@VAR_SOC_NAME has four DRU instances. Two of them belong to the vision and depth
accelerators, and the remaining two are local to the C7x subsystems.

 DRU instance | Base address | Built for
 -------------|--------------|-----------
 VPAC0        | 0x2C200000   | main-r5fss0-0
 DMPAC0       | 0x10200000   | main-r5fss0-0
 C7X256V0     | 0x7C400000   | c75ss0-0
 C7X256V1     | 0x7D400000   | c75ss1-0

A C7x local DRU is only accessible from the C7x that owns it, so a single build
cannot cover all four instances. The example selects its instance table from the
build target, and three builds are needed for full coverage. The main R5F build
covers VPAC0 and DMPAC0, and each C7x build covers its own local DRU.

# Registers Covered {#IP_FMA_DRU_REGISTERS}

The DRU splits its channel registers into a non real time region (CHNRT) holding
the static setup, and a real time region (CHRT) holding control and status that
changes while a transfer is running. Only the static registers are meaningful
for this mechanism. The same split applies to the queue region, where CFG is
static and STATUS is not.

The example checks the following:

* `DRU_QUEUE CFG`, one register for each of the 5 DRU queues. This holds the
  priority, QoS, order ID, consecutive transaction count and re-arbitration wait
  of the queue.
* `DRU_CHNRT CHST_SCHED`, one register for each of the 64 DRU channels. This
  holds the queue that the channel is scheduled on.

# Preconditions and Assumptions

* The board is a @VAR_BOARD_NAME.
* The SBL is built and the board is set to UART boot mode. See
  \ref UART_BOOTLOADER_PYTHON_SCRIPT and \ref BOOTMODE_UART.
* The device owning the DRU registers is powered on before the registers are
  accessed. The example does this with Sciclient for VPAC0 and DMPAC0. The C7x
  local DRUs need no such request because they are part of the C7x subsystem
  that is already running the application.

# Execution Flow

For every DRU instance in the build, the application does the following:

* Requests the TISCI device to the ON state, for the instances that need it. No
  separate reset request is made, because the device manager releases the module
  reset as part of the power on sequence.
* Reads one register from each DRU MMR region, so that an instance that cannot
  be reached is reported before the whole register group is walked.
* Programs the static configuration of all DRU queues and all DRU channels, then
  reads the first queue and channel register back to confirm that the writes
  reached the hardware.
* Captures the expected values of the queue and channel register groups.
* Reads the same groups again and compares them against the expected values.
  This is repeated `DRU_APP_PERIODIC_CHECK_NUM` times with a delay of
  `DRU_APP_PERIODIC_CHECK_DELAY_US` between two readbacks.
* Reports a match or a mismatch for every comparison.

The readback and comparison steps are the safety mechanism itself. The power up
and configuration steps are there to give the mechanism a non default register
pattern to work on.

## Readback Snippet

\code
/* Capture the expected values once, after the DRU has been configured */
IpFma_DruQueueRegs druQueueRegsExpValues;
status = IpFma_Dru_GetQueueRegs(queueBase, &druQueueRegsExpValues);

/* Periodic readback */
IpFma_DruQueueRegs druQueueRegsActualValues;
status = IpFma_Dru_GetQueueRegs(queueBase, &druQueueRegsActualValues);

status = IpFma_Dru_CompareQueueRegs(&druQueueRegsExpValues,
                                    &druQueueRegsActualValues);
if(IPFMA_OK == status)
{
    DebugP_log("Expected - Actual dru queue register values match!\r\n");
}
else
{
    DebugP_log("Expected - Actual dru queue values mismatch!\r\n");
}
\endcode

`queueBase` and the equivalent `chnrtBase` are taken from the `CSL_DRU_t` overlay
placed at the base address of the instance, so the region offsets do not have to
be repeated in the application.

# Build Steps

Build the combination needed. For the main R5F,

\code
cd ${SDK_INSTALL_PATH}
make -s -C examples/ip_fma/dru_static_regs_readback/j722s-evm/main-r5fss0-0_freertos/ti-arm-clang all
\endcode

For the C7x cores, replace the combination with `c75ss0-0_freertos/ti-c7000` or
`c75ss1-0_freertos/ti-c7000`.

The application image `ip_fma_dru.release.appimage.hs_fs` is generated in the
same folder. See \ref MAKEFILE_BUILD_PAGE for the makefile build, or
\ref CCS_PROJECTS_PAGE if CCS projects are used instead.

# Steps to Run the Example

* Point the appimage path in
  `tools/boot/sbl_prebuilt/@VAR_BOARD_NAME_LOWER/default_sbl_uart_hs_fs.cfg` to
  the image that was built.
* Send the SBL and the appimage over UART as described in
  \ref UART_BOOTLOADER_PYTHON_SCRIPT.
* Connect to the UART terminal within 5 seconds of the script finishing to see
  the application logs.

\note Only one appimage is sent per run. To cover all four DRU instances, build
and boot the three combinations one after the other.

# Sample Output

Shown below is the output of the main R5F build, which covers VPAC0 and DMPAC0.

\code
DRU register readback application started ...
DRU instance VPAC0 ..
  Module is ON
  DRU register probe ... Done !!
  DRU queue and channel config ... Done !!
Register check starts ..
Comparing expected-actual dru queue register values...
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Comparing expected-actual dru chnrt register values...
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Register check done ...
DRU instance DMPAC0 ..
  Module is ON
  DRU register probe ... Done !!
Register check starts ..
Comparing expected-actual dru queue register values...
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Expected - Actual dru queue register values match!
Comparing expected-actual dru chnrt register values...
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Expected - Actual dru chnrt register values match!
Register check done ...
All tests have passed!!
\endcode

The configuration step is missing from the DMPAC0 section because that instance
is not programmed by the example. See the limitations below.

# Limitations and Notes

* On @VAR_BOARD_NAME the DMPAC0 DRU registers can be read, but writes to them do
  not take effect. The example does not program DMPAC0 and captures its static
  configuration as it is found, which on a cold boot is the reset value. The
  readback still verifies that this configuration does not change, but it is a
  weaker check than the one done on the instances that can be programmed.
* The C7x local DRU registers cannot be reached from an R5F. Reading them from
  the main R5F stalls the access rather than returning an error, so the C7x
  instances are left out of the R5F instance table instead of being probed and
  skipped at run time.
* The `QUEUE` field of `CHST_SCHED` is 2 bits wide, so a channel can only be
  scheduled on queues 0 to 3 through this register even though the DRU has 5
  queues. The example uses queue 3.
* The mechanism only covers the registers listed in \ref IP_FMA_DRU_REGISTERS.
  Other static DRU registers, such as the channel `CFG` and `CHOES0` registers,
  are written by the example but are not part of the readback.
* The comparison stops at the first register that does not match and reports
  `IPFMA_E_MISMATCH`. It does not report which register differs.

# See Also

\ref IP_FMA_PAGE
