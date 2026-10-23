# Software Read-back of Written Configuration {#SDL_DCC_RB_WRITTEN_CONFIG_PAGE}

[TOC]

# Overview

This safety mechanism (SM) is implemented as a part of the DCC (Dual Clock
Comparator) module as it is intended to be a safety diagnostics test for
this subsystem. It is identified in the TI Functional Safety Manual as the
*Software Read-back of Written Configuration* safety mechanism.

The Dual Clock Comparator (DCC) is used to determine the accuracy of a clock
signal during the execution of an application. Specifically, the DCC is
designed to detect drifts from the expected clock frequency. The desired
accuracy can be programmed based on calculation for each application. The
DCC measures the frequency of a selectable clock source using another input
clock as a reference.

As part of this mechanism, a set of registers used to configure the DCC
module must be read back shortly after being written. The read values are
then compared to the expected values, i.e., the values originally written
into the registers, to verify correct configuration and proper operation of
the module.

This safety mechanism is integrated into the DCC software module. The
safety mechanism code is executed whenever a DCC configuration register is
written through the DCC device abstraction layer APIs. The inclusion of
this safety mechanism is controlled by which SDL library an application
links against: the standard `sdl` library never carries read-back checks
(they are compiled out entirely), while a dedicated `sdl_ipfma` library -
identical to `sdl` in every other respect - always carries them. Section
**Build Steps** below shows how the example application in this SDK links
against `sdl_ipfma` instead of `sdl`.

The register read-back macro used by DCC, `HW_RB_CHECK_FIELD32_VAL`, is not
DCC-specific - it is defined once in
`source/drivers/hw_include/reg_readback.h`, a shared framework used by
several drivers, each opting in independently via a bitmask. A translation
unit identifies itself to the framework by defining
`REG_READBACK_MODULE` to its own bit (`REG_RB_MOD_DCC` for DCC) before
including the header; the mask is compared at compile time against
`REG_READBACK_MODULE_MASK`, so a driver that has not opted in costs nothing
in code size or cycles.

On a mismatch, `HW_RB_CHECK_FIELD32_VAL` invokes the error-handler function
passed as its final argument - `SDL_DCC_readbackErrCbk()`, for every call
site in `sdl_dcc.c`. The macro has no status output of its own: it cannot
report a mismatch back to its caller through a return value. `SDL_DCC_configure`,
`SDL_DCC_enable`, `SDL_DCC_disable`, `SDL_DCC_enableIntr`, and
`SDL_DCC_disableIntr` all still return `SDL_PASS` unconditionally on their
normal (argument-valid) path - a mismatch is visible only through the
callback, not through the calling API's return status. See **Limitations
and Notes** below.

Register writes happen in the file located at
`source/sdl/dcc/v0/sdl_dcc.c`. This is where the register read-back macro
is called.

# Preconditions and Assumptions

Here are some preconditions and assumptions for the correct usage of the SM:

* Use the following TI hardware platform: `J722S` (`mcu-r5fss0-0` core).
* Build and link against the `sdl_ipfma` library (not `sdl`) - see
  **Build Steps** below. An application linked against `sdl` alone never
  has the read-back checks compiled in, regardless of any other setting.

# The Example Application

The example that demonstrates this safety mechanism is located at:

\code
<mcu_plus_sdk>/examples/ip_fma/dcc_readback_of_written_config/
\endcode

This example has **no application source of its own**. It shares the exact
same `main_baremetal.c` as `examples/ip_fma/clk_ip_fma_app` - a full DCC
example that configures `MCU_DCC0`, runs a single-shot clock-drift
measurement, and additionally performs a *periodic* software read-back of the
DCC configuration registers (via the `IpFma_Clk_*` helpers) while it waits for
the DCC completion interrupt.

The two examples are identical in source; the **only** difference is the SDL
library each one links, selected entirely in its own
`.project/project_j722s.js`:

 Example                            | Links       | What runs
 -----------------------------------|-------------|-----------
 `clk_ip_fma_app`                   | `sdl`       | DCC measurement + periodic `IpFma_Clk_*` read-back only
 `dcc_readback_of_written_config`   | `sdl_ipfma` | the same, **plus** every DCC configuration-register write is read back inside the SDL APIs (this SM)

So in the `sdl_ipfma`-linked build the *Software Read-back of Written
Configuration* checks run transparently inside the `SDL_DCC_configure()`,
`SDL_DCC_enable()`, `SDL_DCC_disable()`, `SDL_DCC_enableIntr()` and
`SDL_DCC_disableIntr()` calls that the shared application already makes as part
of its normal DCC flow - no extra application code is needed to exercise them.

Through its `filedirs`, the example compiles the shared sources in place:
`main_baremetal.c` (from `clk_ip_fma_app`), and `ip_fma_clk.c` /
`ip_fma_common.c` (from `source/ip_fma/src`) and `dpl_interface.c` (from
`examples/sdl/dpl`).

The application must also provide the definition of
`SDL_DCC_readbackErrCbk()` - the SDL library declares this callback but
supplies no default, so an application linked against `sdl_ipfma` that omits
it fails to link. It is defined in the shared `main_baremetal.c` and simply
logs any read-back mismatch to the console:

```c
void SDL_DCC_readbackErrCbk(uint32_t expVal, uint32_t readVal)
{
    DebugP_log("DCC read-back callback - register field MISMATCH."
               " Expected: 0x%08X, Read: 0x%08X\r\n", expVal, readVal);
}
```

In the plain-`sdl` `clk_ip_fma_app` build the read-back checks compile out, so
this callback is never referenced - it is harmless dead code there, and lives
in the shared main only so the `sdl_ipfma` build links.

Because the callback only *logs*, this example **reports** a read-back mismatch
but does **not** fold it into its overall pass/fail verdict. The final
`All tests have passed.` (or failure) verdict is driven by the DCC completion
interrupt and the periodic `IpFma_Clk_*` compare, not by the write-read-back
callback. On a healthy device every written register reads back correctly, so
the callback never fires; see **Forcing a Read-back Check Failure** for what a
mismatch looks like, and **Limitations and Notes** for the verdict caveat.

# The sdl_ipfma Library

`sdl_ipfma` is a second SDL library component, generated alongside `sdl`
from `source/sdl/sdl_ipfma/.project/project_j722s.js`. Its file list, include
directories, and compiler flags mirror `sdl` exactly - same source files
from the same shared `source/sdl/` tree - so linking against it in place
of `sdl` changes nothing about the rest of the SDL API surface. The one
difference is that `sdl_ipfma`'s build description always adds:

```
-DREG_READBACK_ENABLED
-DREG_READBACK_MODULE_MASK='(REG_RB_MOD_DCC)'
```

to its compile flags - unconditionally, not driven by any environment
variable. This is why `sdl_dcc.c` still needs to identify itself to the
shared framework:

```c
#define REG_READBACK_MODULE REG_RB_MOD_DCC

#include "sdlr_dcc2.h"
#include "sdl_dcc.h"
```

Without this `#define`, `REG_READBACK_MODULE` would default to
`REG_RB_MOD_NONE` in `reg_readback.h`, and the mask comparison
(`REG_READBACK_MODULE_MASK & REG_READBACK_MODULE`) would always be zero -
so the read-back checks would silently compile to no-ops even inside
`sdl_ipfma`, despite `REG_READBACK_ENABLED` being defined. The two pieces
(module identity in the `.c` file, module mask in the library build) are
both required.

Because `sdl` and `sdl_ipfma` are two separate generated libraries built
from the same source, a single `gen-buildfiles`/`make libs` pass produces
both `sdl.j722s.mcu-r5f.ti-arm-clang.<profile>.lib` and
`sdl_ipfma.j722s.mcu-r5f.ti-arm-clang.<profile>.lib` (read-back always
active for DCC) side by side. Which one an application links against is
decided entirely by that application's own build description, as shown
next.

# Build Steps

**Step 1 - Identify the DCC module to the shared read-back framework
(one-time setup, already done in the SDK).**
`source/drivers/hw_include/reg_readback.h` gates every check on the macro
`REG_READBACK_MODULE`, which defaults to `REG_RB_MOD_NONE`. In
`source/sdl/dcc/v0/sdl_dcc.c`, before the `sdlr_dcc2.h` and `sdl_dcc.h`
include, it is defined as `REG_RB_MOD_DCC` so DCC's register writes
participate in the module-level mask:

```c
#define REG_READBACK_MODULE REG_RB_MOD_DCC

#include "sdlr_dcc2.h"
#include "sdl_dcc.h"
```

**Step 2 - The example links `sdl_ipfma` unconditionally (one-time setup,
already done for this example).**
In `examples/ip_fma/dcc_readback_of_written_config/.project/project_j722s.js`,
the SDL library search path and library name are hardcoded to `sdl_ipfma` -
there is no environment-variable toggle:

```js
const libdirs_nortos_mcu_r5f = {
    common: [
        "${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib",
        "${MCU_PLUS_SDK_PATH}/source/drivers/lib",
        "${MCU_PLUS_SDK_PATH}/source/board/lib",
        "${MCU_PLUS_SDK_PATH}/source/sdl/sdl_ipfma/lib",
    ],
};

const libs_nortos_mcu_r5f = {
    common: [
        "nortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "drivers.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
        "board.j722s.r5f.ti-arm-clang.${ConfigName}.lib",
        "sdl_ipfma.j722s.mcu-r5f.ti-arm-clang.${ConfigName}.lib",
    ],
};
```

The same file's `files`/`filedirs` point at the shared `main_baremetal.c` in
`clk_ip_fma_app` (and at `source/ip_fma/src` and `examples/sdl/dpl`), so this
example builds the identical application, only against `sdl_ipfma`.

**Step 3 - Generate the buildfiles, then build the libraries.**

\code
cd <mcu_plus_sdk>
make gen-buildfiles DEVICE=j722s
make -s libs DEVICE=j722s
\endcode

This generates and builds both SDL libraries:

* `source/sdl/makefile.j722s.mcu-r5f.ti-arm-clang` - plain `sdl`, no
  read-back defines.
* `source/sdl/sdl_ipfma/makefile.j722s.mcu-r5f.ti-arm-clang` - `sdl_ipfma`,
  carrying `-DREG_READBACK_ENABLED` and
  `-DREG_READBACK_MODULE_MASK='(REG_RB_MOD_DCC)'` in its `DEFINES_common`.

It also generates this example's own build files, already wired to link
`sdl_ipfma`.

**Step 4 - Build the example application.**

\code
cd <mcu_plus_sdk>
make -s -C examples/ip_fma/dcc_readback_of_written_config/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/
\endcode

**Step 5 - Verify that the build process completed successfully** by
checking for `dcc_readback_of_written_config.release.appimage.hs_fs` in:

\code
examples/ip_fma/dcc_readback_of_written_config/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/
\endcode

Next, flash the binary on the board to execute the example application.

# Execution Flow

After the build, what remains is to run the program on the board. The
result of building this example application is the appimage binary,
located at the path mentioned in the previous section.

Edit `tools/boot/sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg` to
point at the built appimage:

\code
--file=../../examples/ip_fma/dcc_readback_of_written_config/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/dcc_readback_of_written_config.release.appimage.hs_fs
\endcode

Then flash the board over UART:

\code
cd <mcu_plus_sdk>/tools/boot
python3 uart_bootloader.py -p /dev/ttyUSB2 --cfg=sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg
\endcode

Also bear in mind that this code is prepared and compiled for the
`mcu-r5fss0-0` core, so the application runs on this core which means that
application logs are expected on the configured debug UART interface.

The shared application runs its normal DCC flow: it configures `MCU_DCC0`,
enables it, and while waiting for the completion interrupt it periodically
reads back and compares the DCC configuration registers. Every one of the
`SDL_DCC_*` calls in that flow also performs the write-read-back checks of
this safety mechanism, because the build links `sdl_ipfma`. On a healthy
device those checks all pass silently (the callback never fires), so the
output is the same as the plain periodic run:

\code
DCC Config Registers Periodic Check: Start
DCC Config Registers Periodic Check: Seed values calculation done.
DCC Config Registers Periodic Check: DCC configured
DCC Config Registers Periodic Check: Enabling DCC and waiting for completion interrupt
DCC Config Registers Periodic Check: Register check starts...
DCC Config Registers Periodic Check: Loading expected DCC register values...
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
...
DCC Config Registers Periodic Check: DCC Generated completion interrupt
DCC Config Registers Periodic Check: No Clock Drift was observed
DCC Config Registers Periodic Check: All tests have passed.
DCC Config Registers Periodic Check: Completes!!!
\endcode

The `Comparing.../values MATCH!` pair is the periodic `IpFma_Clk_*` read-back
loop and repeats until the DCC completion interrupt fires. The write-read-back
checks of this SM produce no output on success - they are silent unless a
register reads back a value different from what was written, which is covered
next.

## Forcing a Read-back Check Failure

In the normal execution flow, all read-back values from DCC registers match
the expected values, so a read-back failure never occurs. To force a
read-back failure, a change is required in the library source.

Modify the file `source/sdl/dcc/v0/sdl_dcc.c`, changing one of the
`HW_RB_CHECK_FIELD32_VAL` calls in `SDL_DCC_configure()` to compare
against a value the register can never actually hold, instead of the value
that was just written:

```c
/* Configure DCC mode of operation */
HW_WR_FIELD32(baseAddr + SDL_DCC2_DCCGCTRL, SDL_DCC2_DCCGCTRL_SINGLESHOT,
              pConfig->mode);
// put some impossible value instead of pConfig->mode
HW_RB_CHECK_FIELD32_VAL(baseAddr + SDL_DCC2_DCCGCTRL,
                         SDL_DCC2_DCCGCTRL_SINGLESHOT, 0xFFFFFF, SDL_DCC_readbackErrCbk);
```

`HW_RB_CHECK_FIELD32_VAL` takes the register address, the field name (used
to derive its `_MASK`/`_SHIFT`), the expected value, and an error-handler
function pointer - it has no status output parameter, so a mismatch here
cannot change what `SDL_DCC_configure()` returns; it can only invoke the
handler passed as the last argument.

Because the change is inside the library source, rebuild both the
`sdl_ipfma` library and the example, then reflash:

\code
cd <mcu_plus_sdk>
make -s libs DEVICE=j722s
make -s -C examples/ip_fma/dcc_readback_of_written_config/j722s-evm/mcu-r5fss0-0_nortos/ti-arm-clang/
\endcode

`SDL_DCC_configure()` is called early in the application flow, so on the next
run the read-back callback fires right after start-up and prints the injected
mismatch - **but the run still ends with `All tests have passed.`**:

\code
DCC Config Registers Periodic Check: Start
DCC Config Registers Periodic Check: Seed values calculation done.
DCC read-back callback - register field MISMATCH. Expected: 0x00FFFFFF, Read: 0x00000000
DCC Config Registers Periodic Check: DCC configured
DCC Config Registers Periodic Check: Enabling DCC and waiting for completion interrupt
DCC Config Registers Periodic Check: Register check starts...
DCC Config Registers Periodic Check: Loading expected DCC register values...
DCC Config Registers Periodic Check: Comparing expected-actual DCC register values...
DCC Config Registers Periodic Check: values MATCH!
...
DCC Config Registers Periodic Check: DCC Generated completion interrupt
DCC Config Registers Periodic Check: No Clock Drift was observed
DCC Config Registers Periodic Check: All tests have passed.
DCC Config Registers Periodic Check: Completes!!!
\endcode

Two things to read from this:

* The callback line reports the injected mismatch: the expected value is the
  forced `0x00FFFFFF`, while the "Read" value (`0x00000000` here) reflects what
  the `SINGLESHOT` field genuinely held - confirming the read-back is comparing
  real hardware state, not a stubbed value.
* The overall verdict is **still** `All tests have passed.`. This is expected
  for this example: the write-read-back mismatch is surfaced *only* through the
  callback log line, and this application's pass/fail verdict is derived from
  the DCC completion interrupt and the periodic `IpFma_Clk_*` compare - it does
  not consult the callback. The safety mechanism has done its job (it detected
  and reported the mismatch); acting on that report is left to the integrator.

# Limitations and Notes

The negative (fault) scenario for the register read-back mechanism cannot
be tested without modifying the code. Under normal operation, the hardware
always provides the correct register value, making it impossible to
naturally trigger an incorrect read-back condition. Validating this failure
path therefore requires intentional code changes or fault injection, as
demonstrated in the previous section.

Because `HW_RB_CHECK_FIELD32_VAL` has no status output, a mismatch is only
ever visible through `SDL_DCC_readbackErrCbk()` - `SDL_DCC_configure()`,
`SDL_DCC_enable()`, `SDL_DCC_disable()`, `SDL_DCC_enableIntr()`, and
`SDL_DCC_disableIntr()` all return `SDL_PASS` regardless of whether a
read-back check inside them failed. Any code that checks these APIs'
return status without also implementing `SDL_DCC_readbackErrCbk()` will not
detect a register read-back mismatch through that return status; the callback
is the only mechanism this safety diagnostic provides for surfacing a fault.

This example only **logs** the callback; it does not fail its own test verdict
on a write-read-back mismatch (its `All tests have passed.` verdict comes from
the DCC completion interrupt and the periodic `IpFma_Clk_*` compare). An
integrator who needs a write-read-back mismatch to fail the application must
consume the callback themselves - for example by counting invocations and
gating the final verdict on that count.

Building against the plain `sdl` library instead of `sdl_ipfma` compiles
the read-back checks out entirely, in which case none of the above behavior
applies - `HW_RB_CHECK_FIELD32_VAL` expands to nothing, and
`SDL_DCC_readbackErrCbk()` is never referenced (so, unlike an `sdl_ipfma`
build, an application is not required to define it).
