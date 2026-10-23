# UTC Legal TR Check {#IP_FMA_UTC_LEGAL_TR_CHECK_PAGE}

[TOC]

## Overview

The UTC Legal TR Check example validates DRU hardware error handling by
submitting an unsupported / illegal Transfer Request (TR) and verifying that the
DRU reports the correct error status and reason.

The DRU provides checks on fields of the TR to ensure that it is capable of
performing the requested transaction. This can be used to catch some errors if
the error results in the TR being submitted no longer having legal values.
Depending on the system needs, the system integrator can decide to resubmit TRs
to perform the intended transaction.

This example is intended as a diagnostic and validation application to
demonstrate how software can detect misconfigured TRs using the DRU TR response
mechanism. The application runs in baremetal mode (no OS / RTOS).

On J722S the DRU under test (MSMC_DRU0) is C7x-local and is reached from the main
R5F through the BCDMA_0 UTC path. That path does not expose a NavSS ring, so the
example drives the DRU's own per-channel exposed ring (CHNRT / CHRT MMRs)
directly via the `Udma_chRing*Raw` APIs. No UDMA driver changes are required.

## Preconditions and Assumptions

Here are the preconditions and assumptions for the correct usage of this example:

* Use the following TI hardware platform: J722S.
* VPAC0 must be clocked before the DRU MMRs can be accessed; the application
  enables the module clock for `TISCI_DEV_VPAC0` at startup.
* This example is supported only on the `main-r5fss0-0` core. Execution on other
  cores is not supported.

## Build Steps

To demonstrate the usage of the safety mechanism you need to build the example
application located at
`<mcu_plus_sdk>/examples/ip_fma/dru_unsupported_tr_detection`.

Step 1 - Build the example application. The example is self-contained and is
built for the `main-r5fss0-0` core only:

\code
cd <mcu_plus_sdk>
make -C examples/ip_fma/dru_unsupported_tr_detection/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/
\endcode

Step 2 - Verify the build. Check for `dru_tr_legal.release.appimage.hs_fs` in
`examples/ip_fma/dru_unsupported_tr_detection/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/`.

Step 3 - Flash and run via UART. Edit
`tools/boot/sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg` to point at the
built appimage:

\code
--file=../../examples/ip_fma/dru_unsupported_tr_detection/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/dru_tr_legal.release.appimage.hs_fs
\endcode

Then flash the board over UART:

\code
cd <mcu_plus_sdk>/tools/boot
python3 uart_bootloader.py -p /dev/ttyUSB2 --cfg=sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg
\endcode

## Execution Flow

The following sequence describes how the UTC Legal TR Check test operates.

### 1. Application Initialization

The application initializes the board (pin multiplexing, UART for console
logging) and enables the VPAC0 module clock so the DRU MMRs are accessible. The
UDMA driver is then initialized using default initialization parameters with UTC
support enabled.

### 2. UDMA UTC Channel Configuration

A UDMA UTC channel associated with the DRU instance (MSMC_DRU0) is opened on
BCDMA_0. The channel is opened with `fqRingPrms` so that `Udma_chOpen` configures
the DRU's own exposed ring (CHRING_ADDR). It is configured with
`CSL_DRU_OWNER_DIRECT_TR`, whose `Udma_chConfigUtc` succeeds on the R5F (UDMAC_TR
mode would require SYSFW TX-channel configuration, which is rejected for this
host). While the channel is still disabled, the DRU `CHNRT_CFG` owner bit is
forced to UDMAC_TR by a direct MMR write, since the DRU consumes its forward ring
only in that mode. The channel is then enabled so software can submit TRs.

### 3. DRU Capability Discovery

Before submitting any Transfer Requests, the application reads the DRU
capabilities register using `CSL_druGetCapabilities()`. The following capability
information is printed to the console:

* Supported TR types
* STATIC field support
* EOL field support
* Local and global trigger support
* Supported AMODE values
* Supported ELTYPE values
* Supported DFMT values
* Secure transfer (SECTR) support

When executed, the application prints these capabilities in a format like this:

\code
==== DRU CAPABILITIES ====
Supported TR Types: 8 9 10 11
STATIC support       : YES
EOL support          : NO
Local trigger        : YES
Global trigger       : YES
Supported AMODEs     : 0
Supported ELTYPEs    : 0 1 3
Supported DFMTs      : 0
Secure transfer      : NO
==========================
\endcode

### 4. Unsupported TR Injection

The application intentionally programs a Transfer Request with an unsupported
configuration. In this example the TR is configured as a 3D Data Move TR type,
which is not advertised as supported in the DRU capabilities register.

The TR is packaged into a Transfer Record Packet Descriptor (TRPD) and submitted
to the DRU forward ring (queue + forward doorbell). This TR submission is
expected to be rejected by hardware.

### 5. TR Completion and Error Response Handling

After submission, the application polls the DRU reverse ring occupancy
(`CHRT[ch].RING_RCVS_OCC`), then dequeues the returned descriptor (reverse ring
dequeue + reverse doorbell) and confirms the returned descriptor pointer matches
the submitted TRPD address.

The TR response word written back by hardware into the TRPD is then examined:

* `STATUS_TYPE` is extracted to determine the completion status.
* If an error is reported, `STATUS_INFO` is decoded to identify the specific
  unsupported feature.

For an unsupported TR, the expected response is:

* `STATUS_TYPE` = Unsupported Feature
* `STATUS_INFO` = TR TYPE not supported

For this example, successful execution is defined as correct detection and
reporting of an error, not successful TR completion.

### 6. Result Reporting

The decoded TR status and unsupported reason are printed to the UART console.

### 7. Channel Cleanup

After the test completes:

* The UTC channel is disabled.
* The channel is closed.
* The UDMA driver is de-initialized and the application exits cleanly.

## Result

When the application runs successfully, the console output shows:

* DRU capability information printed at runtime
* Submission of an unsupported TR
* Detection of an error response from hardware
* Correct decoding of the `STATUS_TYPE` and `STATUS_INFO` fields

Example output:

\code
UTC Unsupported TR Check application started...
DRU Capabilities:
==== DRU CAPABILITIES ====
Supported TR Types: 8 9 10 11
STATIC support       : YES
EOL support          : NO
Local trigger        : YES
Global trigger       : YES
Supported AMODEs     : 0
Supported ELTYPEs    : 0 1 3
Supported DFMTs      : 0
Secure transfer      : NO
==========================
TR Status: Unsupported Feature (4)
Unsupported reason: TR TYPE not supported (0)
UTC Unsupported TR Check application done
All tests have passed!!
\endcode
