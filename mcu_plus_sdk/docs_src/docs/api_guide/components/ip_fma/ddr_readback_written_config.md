# DDR – Software Read-back of Written Configuration {#IP_FMA_DDR_RB_WRITTEN_CONFIG_PAGE}

[TOC]

# Overview

This safety mechanism (SM) is implemented as a part of the DDR subsystem, as
it is intended to be a safety diagnostics test for this subsystem. It is
identified in the TI Functional Safety Manual as the *Software Read-back of
Written Configuration* safety mechanism.

The DDR subsystem brings up external memory by programming a large set of
controller, PHY-independent (PI) and PHY configuration registers in the
LPDDR4 memory controller during initialisation. This safety mechanism verifies
that configuration: as part of the mechanism, each register used to configure
the DDR controller/PHY/PI is read back shortly after being written, and the
read value is compared against the expected value, i.e., the value originally
written into the register, to confirm correct configuration of the subsystem.

This safety mechanism is integrated into the DDR controller software module.
The safety mechanism code is executed whenever a DDR configuration register is
written through the LPDDR4 driver's register-write primitive. The inclusion of
this safety mechanism is controlled at build time by two compile defines
(`REG_READBACK_ENABLED` and a module mask selecting `REG_RB_MOD_LPDDR`): a
build that does not define them never carries the read-back checks (they are
compiled out entirely). In this SDK they are applied by the `drivers_ipfma`
library - the read-back-enabled build of the `drivers` library - which the
example application links instead of `drivers`, so the example always carries
the checks. Section **How the Read-back is Enabled in the Example**
below shows how this is wired.

The register read-back macro used by DDR, `RB_CHECK_REG32`, is not DDR-specific
- it is defined once in `source/drivers/hw_include/reg_readback.h`, a shared
framework used by several drivers, each opting in independently via a bitmask.
A translation unit identifies itself to the framework by defining
`REG_READBACK_MODULE` to its own bit (`REG_RB_MOD_LPDDR` for DDR) before
including the header; the mask is compared at compile time against
`REG_READBACK_MODULE_MASK`, so a driver that has not opted in costs nothing in
code size or cycles.

On a mismatch, `RB_CHECK_REG32` invokes the error-handler function passed as its
final argument - `LPDDR4_ddrReadbackErr()`, for every call site in `lpddr4.c`.
The macro has no status output of its own: it cannot report a mismatch back to
its caller through a return value. `LPDDR4_WriteReg()` still returns `CDN_EOK`
on its normal (argument-valid) path - a mismatch is visible only through the
callback, not through the calling API's return status. See **Limitations and
Notes** below.

Register writes happen in the file located at
`source/drivers/ddr/cdn_drv/common/lpddr4.c`, inside `LPDDR4_WriteReg()`. This
is where the register read-back macro is called, once for each of the three
register blocks:

```c
/* Controller registers */
RB_CHECK_REG32(LPDDR4_AddOffset(&(ctlRegBase->DENALI_CTL_0), regOffset),
               regValue, g_lpddr4_ddr_controller_rw_mask[regOffset],
               LPDDR4_ddrReadbackErr);

/* PHY registers */
RB_CHECK_REG32(LPDDR4_AddOffset(&(ctlRegBase->DENALI_PHY_0), regOffset),
               regValue, LPDDR4_getPhyRwMask(regOffset),
               LPDDR4_ddrReadbackErr);

/* PHY-independent (PI) registers */
RB_CHECK_REG32(LPDDR4_AddOffset(&(ctlRegBase->DENALI_PI_0), regOffset),
               regValue, g_lpddr4_pi_rw_mask[regOffset],
               LPDDR4_ddrReadbackErr);
```

The third argument of each call is the register's **read/write mask**: only the
writable bits are compared, so read-only, reserved and observation bits are
excluded from the check. The controller and PI masks are plain lookup tables
(`g_lpddr4_ddr_controller_rw_mask[]`, `g_lpddr4_pi_rw_mask[]`); the PHY mask is
returned by `LPDDR4_getPhyRwMask()`, which selects the correct per-slice mask
for the register's PHY slice (data slice, address slice, or PHY core).

# Preconditions and Assumptions

Here are some preconditions and assumptions for the correct usage of the SM:

* Use the following TI hardware platform: `J722S` (`wkup-r5fss0-0` core).
* The read-back checks must be compiled in - i.e. the translation unit holding
  the register writes (`lpddr4.c`) must be built with `REG_READBACK_ENABLED`
  and `REG_READBACK_MODULE_MASK` including `REG_RB_MOD_LPDDR`. In this SDK this
  is done by the `drivers_ipfma` library (`source/drivers/drivers_ipfma`),
  which the application links *instead of* the `drivers` library. A build that
  omits these never has the read-back checks compiled in, regardless of any
  other setting.
* The mechanism is exercised only while DDR is actually being brought up (see
  **The Example Application**), which on this device happens in the bootloader
  stage. The example is therefore built as a minimal bootloader image.

# The Example Application

The example that demonstrates this safety mechanism is located at:

\code
<mcu_plus_sdk>/examples/ip_fma/ddr_readback_of_written_config/
\endcode

The DDR configuration registers are written by `LPDDR4_WriteReg()`, and the
only path that fans the bulk controller/PHY/PI configuration out to that
function is the DDR bring-up sequence in `source/drivers/ddr/v1/ddr.c`:

```c
isEnabled = DDR_isEnabled(prm);

if (!isEnabled)
{
    status = DDR_initHwRegs(prm);   /* writes the config -> LPDDR4_WriteReg */
    ...
}
```

`DDR_isEnabled()` returns true once the DDR controller is already running
(both the `PI_START` and `START` bits are set). Consequently, the configuration
writes - and therefore this read-back safety mechanism - run **only during the
first DDR bring-up**, when the controller is not yet started. On this device
that bring-up is performed by the bootloader; any ordinary application is
started *after* the bootloader has already brought DDR up, so by the time such
an application calls `DDR_init()` the controller is enabled, the configuration
writes are skipped, and the read-back never fires.

For that reason the example is built as a **minimal bootloader image**: it is
among the first code the ROM runs, before DDR has been brought up, so
`DDR_isEnabled()` is false and the configuration writes (and the read-back) are
genuinely executed. The example has a single application source,
`main_baremetal.c`, which:

* calls `System_init()` - this is where `DDR_init()` runs and the read-back
  fires on every controller/PHY/PI register write;
* then opens the console UART (`Board_init()` / `Drivers_open()` /
  `Board_driversOpen()`) and reports the result;
* then performs the standard teardown and returns (this bootloader boots
  nothing further - it exists solely to demonstrate the SM).

The read-back runs inside `System_init()`, **before** the console UART is open,
so the mismatch callback cannot log directly. Instead the application's callback
only *records* (a mismatch count plus the first expected/read values), and
`main()` reports the recorded result once the UART is up:

```c
void LPDDR4_ddrReadbackErr(uint32_t expVal, uint32_t readVal)
{
    if (gDdrWrRbMismatchCount == 0U)
    {
        gDdrWrRbFirstExp  = expVal;
        gDdrWrRbFirstRead = readVal;
    }
    gDdrWrRbMismatchCount++;
}
```

Unlike a callback that only logs, this example **folds the result into its own
pass/fail verdict**: `main()` prints `All tests have passed!` only when the
recorded mismatch count is zero, and `Some tests have failed` otherwise. On a
healthy device every written register reads back correctly, so the callback
never fires and the count stays zero; see **Forcing a Read-back Check Failure**
for what a mismatch looks like.

# How the Read-back is Enabled in the Example

The LPDDR4 driver source `lpddr4.c` is normally compiled into the shared
`drivers` library *without* the read-back defines, so simply adding
`-DREG_READBACK_ENABLED` to the example's compile flags would only affect the
example's own translation units, not the prebuilt `lpddr4.obj` inside the
`drivers` library.

To get the read-back-enabled version linked, the SDK provides a second build of
the driver library, **`drivers_ipfma`**, located at
`source/drivers/drivers_ipfma`. It exists in parallel to `drivers` (the same
pattern as `sdl` / `sdl_ipfma`): it compiles exactly the same source files, with
the same compiler settings, as the `drivers` library for the given core, and
additionally defines the read-back switches. An application links **either**
`drivers` **or** `drivers_ipfma`, never both.

For the `wkup-r5f` core (on which this example runs) the library is built by
the makefile `source/drivers/drivers_ipfma/makefile.j722s.wkup-r5f.ti-arm-clang`,
whose `DEFINES_common` section carries the read-back switches in addition to the
defines used by `drivers`:

\code
DEFINES_common := \
    -DSOC_J722S \
    -DWKUP_R5 \
    -DENABLE_SCICLIENT_DIRECT \
    -DENABLE_PHY_TUNING_SOC_BUILD \
    -DCONFIG_UDMAP_MULTI_CH_CFG \
    -DREG_READBACK_ENABLED \
    -DREG_READBACK_MODULE_MASK='(REG_RB_MOD_LPDDR)' \
    -DSW_VERSION=$(SW_VERSION)
\endcode

The library produced is
`drivers_ipfma.j722s.wkup-r5f.ti-arm-clang.<profile>.lib` in
`source/drivers/drivers_ipfma/lib`. Only the translation units that identify
themselves to the read-back framework with a module bit contained in the mask
(here `lpddr4.c`, `REG_RB_MOD_LPDDR`) get the read-back checks; every other
object in the library is identical to its counterpart in `drivers`.

The example links `drivers_ipfma` **in place of** the `drivers` library, as can
be seen in the `LIBS_PATH_common` and `LIBS_common` sections of the example's
makefile
(`examples/ip_fma/ddr_readback_of_written_config/j722s-evm/wkup-r5fss0-0_nortos/ti-arm-clang/makefile`):

\code
LIBS_PATH_common = \
	-Wl,-i${MCU_PLUS_SDK_PATH}/source/kernel/nortos/lib \
	-Wl,-i${MCU_PLUS_SDK_PATH}/source/drivers/drivers_ipfma/lib \
	...

LIBS_common = \
	-lnortos.j722s.r5f.ti-arm-clang.${ConfigName}.lib \
	-ldrivers_ipfma.j722s.wkup-r5f.ti-arm-clang.${ConfigName}.lib \
	...
\endcode

As a result the driver's own `DDR_init() -> LPDDR4_WriteReg()` path uses the
read-back-enabled `lpddr4.obj`. This can be confirmed in the example's linker
map file, where all `lpddr4.obj` sections are listed as coming from
`drivers_ipfma.j722s.wkup-r5f.ti-arm-clang.<profile>.lib`.

For the checks to be active, `lpddr4.c` must also identify itself to the shared
framework. It defines its module bit before including the read-back header:

```c
#define REG_READBACK_MODULE  REG_RB_MOD_LPDDR
#include <drivers/hw_include/reg_readback.h>
```

Without this `#define`, `REG_READBACK_MODULE` would default to
`REG_RB_MOD_NONE`, and the mask comparison
(`REG_READBACK_MODULE_MASK & REG_READBACK_MODULE`) would always be zero - so the
read-back checks would silently compile to no-ops even with
`REG_READBACK_ENABLED` defined. The two pieces (module identity in the `.c`
file, module mask in the `drivers_ipfma` library build) are both required.

## Disabling the Read-back for a Module

The modules that carry the read-back checks in `drivers_ipfma` are selected
only by `REG_READBACK_MODULE_MASK` in the library's makefile. To disable the
read-back for a particular module, remove that module's bit from the mask in
the `DEFINES_common` section of the makefile for the affected core, then
rebuild the library and the application.

For DDR, edit `source/drivers/drivers_ipfma/makefile.j722s.wkup-r5f.ti-arm-clang`.
`REG_RB_MOD_LPDDR` is the only bit in the `wkup-r5f` mask, so the mask becomes
`REG_RB_MOD_NONE`:

\code
DEFINES_common := \
    ...
    -DREG_READBACK_ENABLED \
    -DREG_READBACK_MODULE_MASK='(REG_RB_MOD_NONE)' \
    -DSW_VERSION=$(SW_VERSION)
\endcode

When the mask selects several modules, remove only the bit of the module being
disabled and keep the others. For example,
`'(REG_RB_MOD_DSS | REG_RB_MOD_LPDDR)'` becomes `'(REG_RB_MOD_DSS)'`.

Then clean and rebuild the library, and rebuild the example:

\code
cd <mcu_plus_sdk>
make -s -f makefile.j722s drivers_ipfma_wkup-r5f.ti-arm-clang_clean
make -s -f makefile.j722s drivers_ipfma_wkup-r5f.ti-arm-clang
make -s -C examples/ip_fma/ddr_readback_of_written_config/j722s-evm/wkup-r5fss0-0_nortos/ti-arm-clang/
\endcode

The clean step is required. The library objects do not depend on the makefile
itself, so without it the existing `lpddr4.obj`, which still contains the
checks, is archived again unchanged.

With `REG_RB_MOD_LPDDR` removed from the mask, the `RB_CHECK_REG32` checks in
`lpddr4.c` are folded away at compile time and `LPDDR4_ddrReadbackErr()` is
never called. The example
then always reports `0 mismatch(es)` and `All tests have passed!`, but this
verdict no longer reflects any check. To re-enable the read-back, restore
`'(REG_RB_MOD_LPDDR)'` in the mask and repeat the clean and rebuild.

# Build Steps

**Step 1 - Identify the DDR module to the shared read-back framework (one-time
setup, already done in the SDK).** In
`source/drivers/ddr/cdn_drv/common/lpddr4.c`, before the read-back header is
included, the module bit is defined:

```c
#define REG_READBACK_MODULE  REG_RB_MOD_LPDDR
#include <drivers/hw_include/reg_readback.h>
```

**Step 2 - The `drivers_ipfma` library is built with the read-back defines
(one-time setup, already done in the SDK).** As shown in **How the Read-back is
Enabled in the Example**, the `DEFINES_common` section of
`source/drivers/drivers_ipfma/makefile.j722s.wkup-r5f.ti-arm-clang` contains
`REG_READBACK_ENABLED` and `REG_READBACK_MODULE_MASK='(REG_RB_MOD_LPDDR)'` in
addition to the `drivers` defines, and the example links this library instead
of the `drivers` library. There is no environment-variable toggle.

**Step 3 - Build the libraries.** This builds all SDK libraries, including
`drivers_ipfma`:

\code
cd <mcu_plus_sdk>
make -s libs DEVICE=j722s
\endcode

The `drivers_ipfma` library alone can also be (re)built with:

\code
cd <mcu_plus_sdk>
make -s -f makefile.j722s drivers_ipfma_wkup-r5f.ti-arm-clang
\endcode

**Step 4 - Build the example application.**

\code
cd <mcu_plus_sdk>
make -s -C examples/ip_fma/ddr_readback_of_written_config/j722s-evm/wkup-r5fss0-0_nortos/ti-arm-clang/
\endcode

**Step 5 - Verify that the build process completed successfully** by checking
for `ddr_readback_of_written_config.release.hs_fs.tiimage` in:

\code
examples/ip_fma/ddr_readback_of_written_config/j722s-evm/wkup-r5fss0-0_nortos/ti-arm-clang/
\endcode

Next, boot the binary on the board to execute the example application.

# Execution Flow

After the build, what remains is to run the program on the board. The result of
building this example is a bootloader image (`.hs_fs.tiimage`), located at the
path mentioned in the previous section. Because the image *is* the bootloader
(and boots nothing further), it is loaded directly over the ROM UART-boot path
rather than as an application image on top of a separate SBL.

Point `tools/boot/sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg` at the
built image, as the bootloader, with **no** application image (this example
boots nothing, so the `--file=` line is omitted):

\code
--bootloader=../../examples/ip_fma/ddr_readback_of_written_config/j722s-evm/wkup-r5fss0-0_nortos/ti-arm-clang/ddr_readback_of_written_config.release.hs_fs.tiimage
# --file= line intentionally omitted: this example boots nothing
\endcode

Set the board boot-mode switches to UART boot and power-cycle it, then run the
boot script (start it within a second or two of the power-cycle so it catches
the ROM's handshake):

\code
cd <mcu_plus_sdk>/tools/boot
python3 uart_bootloader.py -p /dev/ttyUSB2 --cfg=sbl_prebuilt/j722s-evm/default_sbl_uart_hs_fs.cfg
\endcode

Also bear in mind that this code is prepared and compiled for the
`wkup-r5fss0-0` core, so the application runs on this core, which means that
application logs are expected on the configured debug UART interface
(`WKUP_USART0`).

The image brings DDR up: `System_init()` runs `DDR_init()`, which (DDR not yet
started) applies the controller/PHY/PI configuration, and every write is read
back and compared by this safety mechanism. On a healthy device those checks all
pass, the callback never fires, and `main()` reports a clean result:

\code
DDR Software Read-back of Written Configuration
DDR write-config read-back: 0 mismatch(es) during DDR init
All DDR config registers verified. All tests have passed!
\endcode

The write-read-back checks of this SM produce no per-register output on success
- they are silent unless a register reads back a value different from what was
written, which is covered next.

## Forcing a Read-back Check Failure

In the normal execution flow, all read-back values from DDR registers match the
expected values, so a read-back failure never occurs. To force a read-back
failure, a change is required in the driver source.

Modify `source/drivers/ddr/cdn_drv/common/lpddr4.c`, changing one of the
`RB_CHECK_REG32` calls in `LPDDR4_WriteReg()` to compare against a value the
register can never actually hold, instead of the value that was just written -
for example, compare the controller register against `0` instead of `regValue`:

```c
CPS_REG_WRITE(LPDDR4_AddOffset(&(ctlRegBase->DENALI_CTL_0), regOffset), regValue);
/* force a mismatch: compare against 0 instead of the value just written */
RB_CHECK_REG32(LPDDR4_AddOffset(&(ctlRegBase->DENALI_CTL_0), regOffset),
               0U, g_lpddr4_ddr_controller_rw_mask[regOffset],
               LPDDR4_ddrReadbackErr);
```

`RB_CHECK_REG32` takes the register address, the expected value, the read/write
mask, and an error-handler function pointer - it has no status output
parameter, so a mismatch here cannot change what `LPDDR4_WriteReg()` returns; it
can only invoke the handler passed as the last argument.

Because the change is inside the source built into the `drivers_ipfma` library,
rebuild the library first, then rebuild the example and re-boot it:

\code
cd <mcu_plus_sdk>
make -s -f makefile.j722s drivers_ipfma_wkup-r5f.ti-arm-clang
make -s -C examples/ip_fma/ddr_readback_of_written_config/j722s-evm/wkup-r5fss0-0_nortos/ti-arm-clang/
\endcode

On the next run the read-back callback fires during DDR bring-up and `main()`
reports the mismatch - and, because this example gates its verdict on the
recorded mismatch count, the run ends with `Some tests have failed`:

\code
DDR Software Read-back of Written Configuration
DDR write-config read-back: 1 mismatch(es) during DDR init
  first mismatch: expected 0x00000000, read 0x00000B00
Some tests have failed
\endcode

Two things to read from this:

* The report line reflects the injected mismatch: the expected value is the
  forced `0x00000000`, while the "Read" value (`0x00000B00` here) reflects what
  the register genuinely held after the write - confirming the read-back is
  comparing real hardware state, not a stubbed value.
* The overall verdict is `Some tests have failed`, because this example consumes
  the callback (it counts invocations) and gates its final verdict on that
  count. The safety mechanism has done its job - it detected and reported the
  mismatch.

# Limitations and Notes

The negative (fault) scenario for the register read-back mechanism cannot be
tested without modifying the code. Under normal operation, the hardware always
provides the correct register value, making it impossible to naturally trigger
an incorrect read-back condition. Validating this failure path therefore
requires intentional code changes or fault injection, as demonstrated in the
previous section.

This safety mechanism is exercised only during the first DDR bring-up, i.e. in
the bootloader stage, because the configuration writes it checks are performed
only when `DDR_isEnabled()` is false. An application that runs after DDR has
already been brought up does not re-run those writes and therefore does not
exercise this mechanism - which is why the example is a bootloader image. (DDR
registers are also touched at runtime by other driver paths, such as a single
`BIST_GO` write from the DDR ISR, but those are not the configuration bring-up
and are not the subject of this mechanism.)

The read-back runs inside `System_init()`, before the console UART is open, so a
mismatch cannot be logged at the moment it is detected. The example works around
this by having its callback only *record* the result and having `main()` report
it once the UART is up.

Because `RB_CHECK_REG32` has no status output, a mismatch is only ever visible
through `LPDDR4_ddrReadbackErr()` - `LPDDR4_WriteReg()` returns `CDN_EOK`
regardless of whether a read-back check inside it failed. Any code that checks
this API's return status without also implementing `LPDDR4_ddrReadbackErr()`
will not detect a register read-back mismatch through that return status; the
callback is the only mechanism this safety diagnostic provides for surfacing a
fault. The driver supplies a default (weak) empty `LPDDR4_ddrReadbackErr()`, so
a build that does not care about mismatches still links; an application that
wants to act on them provides its own definition, as this example does.

The read-back compares only the writable bits of each register, using the
register's read/write mask (the controller/PI mask tables and, for PHY
registers, `LPDDR4_getPhyRwMask()`). Read-only, reserved and observation bits
are masked out of the comparison, so they do not produce spurious mismatches.

Building the DDR driver without `REG_READBACK_ENABLED` (as the plain `drivers`
library does) compiles the read-back checks out entirely, in which case none of
the above behaviour applies - `RB_CHECK_REG32` expands to nothing.
