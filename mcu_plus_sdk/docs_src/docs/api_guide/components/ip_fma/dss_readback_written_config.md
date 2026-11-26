# DSS - Software Read-back of Written Configuration {#IP_FMA_DSS_RB_WRITTEN_CONFIG_PAGE}

[TOC]

# Overview

This safety mechanism (SM) is implemented as a part of the DSS (Display
Subsystem) module as it is intended to be a safety diagnostics test for this
subsystem. It is identified in the TI Functional Safety Manual as the
*Software Read-back of Written Configuration* safety mechanism.

The Display Subsystem (DSS) drives the display pipeline - it fetches pixel
data from memory through its video pipes, blends it in the overlay manager,
and streams it out through a video port to the attached display. A set of
common configuration registers control subsystem-wide behaviour such as the
CBA (Common Bus Architecture) read-request priorities.

As part of this mechanism, a configuration register written while the DSS is
being set up is read back shortly after being written. The read value is then
compared to the expected value, i.e., the value originally written into the
register, to verify correct configuration of the module.

This safety mechanism is integrated into the DSS common CSL layer. The
safety-mechanism code runs whenever the instrumented DSS configuration register
is written through the DSS CSL API. Its inclusion is controlled at compile time:
the standard `drivers` library never carries the read-back check (it is compiled
out entirely). The read-back-enabled build is kept in a **separate library**,
`drivers_ipfma` (`source/drivers/drivers_ipfma`), built in parallel to `drivers`:
it compiles exactly the same sources, defines and flags as `drivers`, plus
`REG_READBACK_ENABLED` and a `REG_READBACK_MODULE_MASK` selecting the modules
whose read-back is enabled (`REG_RB_MOD_DSS` for `main-r5f`). An application
links **either** `drivers` **or** `drivers_ipfma`, never both. A **separate
example**, `dss_readback_of_written_config`, links `drivers_ipfma` instead of
`drivers`. Section **Build Steps** below describes how to build and run both.

The register read-back macro used by DSS, `RB_CHECK_REG32`, is not DSS-specific
- it is defined once in `source/drivers/hw_include/reg_readback.h`, a shared
framework used by several drivers, each opting in independently via a bitmask. A
translation unit identifies itself to the framework by defining
`REG_READBACK_MODULE` to its own bit (`REG_RB_MOD_DSS` for DSS) before including
the header; the mask is compared at compile time against
`REG_READBACK_MODULE_MASK`, so a driver that has not opted in costs nothing in
code size or cycles.

On a mismatch, `RB_CHECK_REG32` invokes the error-handler function passed as its
final argument - `CSL_DSSReadbackErr()`. The macro has no status output of its
own: it cannot report a mismatch back to its caller through a return value.
`CSL_dssSetCbaConfig()` returns `void`, so a mismatch is visible only through the
callback. See **Limitations and Notes** below.

The register write and its read-back are in
`source/drivers/dss/v1/hw_include/V3/csl_dssCommon.c`, in `CSL_dssSetCbaConfig()`.
This is the only part of the mechanism that ships in the SDK:

```c
#define REG_READBACK_MODULE  REG_RB_MOD_DSS
#include <drivers/hw_include/reg_readback.h>
...
CSL_REG32_WR(&commRegs->DSS_CBA_CFG, regVal);
RB_CHECK_REG32(&commRegs->DSS_CBA_CFG, regVal, RB_MASK_ALL_BITS_32,
               CSL_DSSReadbackErr);
```

`CSL_DSSReadbackErr()` is declared by the CSL but has **no default
implementation**: an application built with the read-back enabled must define
it, otherwise the link fails. The standard `drivers` library compiles the check out,
so the callback is never referenced there and no definition is required.
`CSL_dssSetCbaConfig()` is called during DSS controller bring-up (from
the dctrl init path in `source/drivers/dss/v1/dctrl/dss_dctrlApi.c`), so the
check runs as a natural part of any DSS initialisation - no extra application
call is needed to exercise it.

# Preconditions and Assumptions

Here are some preconditions and assumptions for the correct usage of the SM:

* Use the following TI hardware platform: `J722S` (`main-r5fss0-0` core - the
  DSS driver runs in the main domain).
* The read-back check is **not** compiled into the prebuilt `drivers` library.
  It is built separately in the `drivers_ipfma` library and exercised by the
  `dss_readback_of_written_config` example, both described below.

# Build Steps

To demonstrate the usage of the safety mechanism you need to build the example
application located at
`<mcu_plus_sdk>/examples/ip_fma/dss_readback_of_written_config`.

Step 1 - Build the `drivers_ipfma` library for `main-r5f` (it is also built as
part of `make -s libs DEVICE=j722s`):

\code
cd <mcu_plus_sdk>
make -s -f makefile.j722s drivers_ipfma_main-r5f.ti-arm-clang
\endcode

Step 2 - Build the example application. It is built for the `main-r5fss0-0` core
only, and links the `drivers_ipfma` library instead of `drivers`:

\code
cd <mcu_plus_sdk>
make -C examples/ip_fma/dss_readback_of_written_config/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/
\endcode

Step 3 - Verify the build. Check for
`dss_readback_of_written_config.release.appimage.hs_fs` in
`examples/ip_fma/dss_readback_of_written_config/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/`.

Step 4 - Flash and run via UART. Edit
`tools/boot/sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg` to point at the
built appimage:

\code
--file=../../examples/ip_fma/dss_readback_of_written_config/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/dss_readback_of_written_config.release.appimage.hs_fs
\endcode

Then flash the board over UART:

\code
cd <mcu_plus_sdk>/tools/boot
python3 uart_bootloader.py -p /dev/ttyUSB2 --cfg=sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg
\endcode

# Execution Flow

After flashing and running the appimage (Build Steps above), the application
runs its normal DSS display-test flow: it brings the DSS up - which calls
`CSL_dssSetCbaConfig()`,
and therefore performs the write-read-back check of this safety mechanism - and
then runs the display test. On a healthy device the check passes silently (the
callback never fires), so the output is the normal `dss_display_test` output
with no read-back line:

\code
DispApp_init() - DONE !!!
DSS display application started...
Display create complete!!
Starting display ... !!!
Display in progress ... DO NOT HALT !!!
...
DSS display test Passed!!
\endcode

The write-read-back check of this SM produces no output on success - it is
silent unless the register reads back a value different from what was written,
which is covered next.

> **Note on the display.** `dss_display_test` is a display example and expects an
> external display for its *display* stage (the j722s build targets **HDMI** -
> `dss1.selectDisplayInterface = "HDMI"`). The read-back SM, however, does
> **not** depend on a connected display: `CSL_dssSetCbaConfig()` runs inside the
> `IOCTL_DSS_DCTRL_SET_GLOBAL_DSS_PARAMS` handler during DSS *configuration*,
> before any pixels are streamed, and the example continues through that
> configuration even if the external display/bridge setup reports a failure. So
> the read-back check (and its callback) fires whether or not a monitor is
> attached; a display only matters if you want the full display test to stream
> frames and pass its display stage.

## Forcing a Read-back Check Failure

In the normal execution flow, the read-back value always matches the expected
value, so a read-back failure never occurs. To force one, change the expected
value passed to `RB_CHECK_REG32` in
`source/drivers/dss/v1/hw_include/V3/csl_dssCommon.c`, in `CSL_dssSetCbaConfig()`,
to a value the register can never actually hold, instead of the value that was
just written (`regVal`):

```c
CSL_REG32_WR(&commRegs->DSS_CBA_CFG, regVal);
/* put an impossible expected value instead of regVal */
RB_CHECK_REG32(&commRegs->DSS_CBA_CFG, 0xFFFFU, RB_MASK_ALL_BITS_32,
               CSL_DSSReadbackErr);
```

`RB_CHECK_REG32` takes the register address, the expected value, the comparison
mask, and an error-handler function pointer - it has no status output parameter,
so a mismatch here cannot change what `CSL_dssSetCbaConfig()` does; it can only
invoke the handler passed as the last argument.

Because `csl_dssCommon.c` is compiled into the `drivers_ipfma` library, rebuild
that library and then the example, and reflash:

\code
cd <mcu_plus_sdk>
make -s -f makefile.j722s drivers_ipfma_main-r5f.ti-arm-clang
make -C examples/ip_fma/dss_readback_of_written_config/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang/
\endcode

`CSL_dssSetCbaConfig()` runs during DSS bring-up, so on the next run the
read-back callback fires and prints the injected mismatch:

\code
DSS display application started...
Call of the read-back error callback function
DSS read-back: register MISMATCH. Expected: 0x0000FFFF, Read: 0x0000000C
Display create complete!!
...
DSS display test Passed!!
\endcode

The "Expected" value is the forced `0x0000FFFF`, while the "Read" value
(`0x0000000C` here) reflects what `DSS_CBA_CFG`
genuinely holds - confirming the read-back is comparing real hardware state, not
a stubbed value. Remember to revert this change after testing.

# Limitations and Notes

The negative (fault) scenario for the register read-back mechanism cannot be
tested without modifying the code. Under normal operation the hardware always
provides the correct register value, making it impossible to naturally trigger
an incorrect read-back condition. Validating this failure path therefore
requires intentional code changes or fault injection, as demonstrated above.

Because `RB_CHECK_REG32` has no status output, a mismatch is only ever visible
through `CSL_DSSReadbackErr()` - `CSL_dssSetCbaConfig()` returns `void`. Any code
that relies on API return status alone will not detect a register read-back
mismatch; the callback is the only mechanism this safety diagnostic provides for
surfacing a fault.

The example override shown above only **logs** the callback; it does not fail the
display test's verdict on a mismatch. An integrator who needs a mismatch to fail
the application must consume the callback themselves - for example by counting
invocations and gating the final verdict on that count.

Building the standard `drivers` library (without the `REG_READBACK_*` defines)
compiles the read-back check out entirely, in which case none of the above
behaviour applies - `RB_CHECK_REG32` expands to nothing, and
`CSL_DSSReadbackErr()` is never referenced.
