# DRU – Basic Functionality Test with Information Redundancy Check {#IP_FMA_DRU_BASIC_TESTS_WITH_CRC_CHECK}

[TOC]

# Introduction

The Data Routing Unit (DRU) is a block copy DMA engine. Software hands it a
Transfer Request (TR) holding a source address, a destination address and the
shape of the move, and the DRU performs the read and the write itself.

This page covers a software diagnostic built out of two parts. The first is a
test of the basic functionality of the engine: buffers are set up in memory, TRs
are built and submitted, and the data that arrives at the destination is checked
against what was sent. The addresses of those buffers are picked so that every
address line of the bus is driven both low and high over the course of the test.
The second part applies an information redundancy technique to the moved data.
A CRC-32 signature is computed over the payload and embedded in the buffer
before it is transferred. On the destination side the signature is extracted,
recomputed from the received payload and compared against the embedded value,
so that the integrity of the transferred data is established independently of
the completion status reported by the hardware.

The example is located at
`${SDK_INSTALL_PATH}/examples/ip_fma/dru_basic_tests_with_crc_check`. It runs on
the main R5F and drives the DRU of the VPAC0 module, which the UDMA driver
exposes as a UTC instance of BCDMA_0.

# Supported Combinations {#IP_FMA_DRU_BASIC_TESTS_COMBOS}

\cond SOC_J722S

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | main-r5fss0-0 nortos
 Toolchain      | ti-arm-clang
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | examples/ip_fma/dru_basic_tests_with_crc_check

\endcond

# Test Buffers and Address Line Coverage {#IP_FMA_DRU_BASIC_TESTS_BUFFERS}

A pair of addresses that are bitwise inverses of each other drives every address
line to both values, which is the property the test is after. On @VAR_SOC_NAME
the DDR window runs from 0x8000_0000 to 0xFFFF_FFFF, so 0x5555_5555 is not a DDR
address at all. The example uses 0x9555_5555 in its place: it keeps the
alternating 0x5555_5555 pattern on address lines 0 to 29 and has bit 31 set so
that the address lands in DDR.

 Address     | Memory    | Notes
 ------------|-----------|-------
 0x9555_5555 | DDR       | Alternating pattern on lines 0 to 29, bit 31 set
 0xAAAA_AAAA | DDR       | Inverse of the above over lines 0 to 29
 0x7910_2A00 | MCU MSRAM | `CSL_MCU_MSRAM_256K0_RAM_BASE` + 0x2A00, bit 31 clear

The two DDR addresses differ in lines 0 to 29 only, since both of them hold bit
31 at 1 and bit 30 at 0:

    0x95555555 ^ 0xAAAAAAAA = 0x3FFFFFFF

Nothing in DDR can drive bit 31 low, so the top two lines are covered by a
second pair of transfers against a buffer in the MCU MSRAM:

    0x95555555 ^ 0x79102A00 = 0xEC457F55

Four transfers are run in total. Each destination becomes the source of the next
transfer, and every destination is cleared before its transfer, so the data has
to be moved back and forth by the DRU instead of being found where it already
was.

 Transfer | Source      | Destination | Lines newly driven
 ---------|-------------|-------------|--------------------
 1        | 0x9555_5555 | 0xAAAA_AAAA | 0 to 29
 2        | 0xAAAA_AAAA | 0x9555_5555 | 0 to 29, other direction
 3        | 0x9555_5555 | 0x7910_2A00 | 30 and 31
 4        | 0x7910_2A00 | 0x9555_5555 | 30 and 31, other direction

# Buffer Layout

All three buffers are 192 bytes and share the same layout.

 Offset | Size | Contents
 -------|------|----------
 0      | 124  | Payload, a ramp 0, 1, 2, ... on the source side
 124    | 4    | CRC-32 signature of the payload
 128    | 64   | Guard region on a destination, tail pattern on a source

Only the first 128 bytes take part in a transfer. That is twice the 64 byte
burst size of the DRU, so a move always spans more than one burst.

The 64 bytes behind the data are what covers the length logic of the engine, and
the two ends use different patterns for it. The guard region of a destination is
written with 0x5A, and the region behind the data of a source is written with
0xA5. A transfer that runs past its length reads the tail of the source and
writes it into the guard region of the destination, so the guard check finds
0xA5 where it expects 0x5A and reports the overrun. Were both ends to use the
same value, that write would land unnoticed.

Both regions are refreshed before every transfer, because the buffers change
roles as the test walks through the address pairs and a buffer that has just
been a destination carries the guard pattern behind its data.

The signature is a CRC-32 (IEEE 802.3) computed in software by `App_swCrc32()`,
using the reflected form of the polynomial:

 Parameter     | Value
 --------------|-------
 Polynomial    | 0x04C11DB7, reflected as 0xEDB88320
 Initial value | 0xFFFFFFFF
 Final XOR     | 0xFFFFFFFF

No MCRC peripheral or other accelerator is involved, which keeps the check
independent of the hardware being tested. Because the payload is a fixed ramp,
the signature is always 0x545A74C0 and can be compared across runs.

# TR Submission Path

The DRU is reached through the UDMA driver as UTC instance
`UDMA_UTC_ID_MSMC_DRU0` of BCDMA_0, and `enableUtc` has to be set in the UDMA
init parameters or the channel cannot be opened at all. The channel is opened
with ring memory supplied in `chPrms.fqRingPrms`, which makes the driver program
the ring that belongs to the DRU channel itself, in `CHNRT CHRING_ADDR` and
`CHNRT SIZE`. Transfer requests are posted to that ring rather than to a ring of
the DMA subsystem.

The channel is configured with `CSL_DRU_OWNER_DIRECT_TR`. The DRU only fetches
from its forward ring when the owner field of `CHNRT CFG` says UDMAC TR, so that
bit is set with a direct register write while the channel is still disabled. The
owner bit and the channel to queue mapping are both read back before the channel
is enabled.

\code
chCfg  = CSL_REG64_RD(&pDruRegs->CHNRT[chNum].CFG);
chCfg |= (uint64_t) CSL_DRU_CHNRT_CFG_CHAN_TYPE_OWNER_MASK;
CSL_REG64_WR(&pDruRegs->CHNRT[chNum].CFG, chCfg);
\endcode

The TR type matters as well. This DRU supports types 8 to 11, and the example
uses type 9 with linear addressing, a one byte element type and EOL disabled.
Its TR record is 64 bytes, so the TRPD is laid out as a 64 byte descriptor
header, one TR and a four byte TR response.

A transfer is then submitted to the ring, and completion is taken from the
reverse ring occupancy of the channel rather than from an interrupt.

\code
retVal = Udma_chRingQueueRaw(chHandle, (uint8_t *)(uintptr_t) trpdPhys, 1U);
retVal = Udma_chRingRingDbRaw(chHandle, 1U);

do
{
    occ = CSL_REG64_RD(&pDruRegs->CHRT[chNum].RING_RCVS_OCC);
    timeoutCnt--;
} while((0ULL == (occ & occMask)) && (timeoutCnt > 0U));

(void) Udma_chRingDeQueueRaw(chHandle, 1U, &respElem);
(void) Udma_chRingRingRvrDbRaw(chHandle, 1U);
\endcode

The descriptor pointer that comes back has to match the one submitted, and the
TR response written into the TRPD has to read
`CSL_UDMAP_TR_RESPONSE_STATUS_COMPLETE`. Only then are the contents of the
destination buffer looked at.

# Preconditions and Assumptions

* The board is a @VAR_BOARD_NAME.
* The VPAC0 module clock is enabled before any DRU register is accessed. The
  example does this with `SOC_moduleClockEnable()`.
* UTC support is enabled in the UDMA init parameters, otherwise the DRU channels
  of BCDMA_0 cannot be opened.
* The three buffer addresses listed in \ref IP_FMA_DRU_BASIC_TESTS_BUFFERS are
  written directly and are assumed to belong to no other software in the system.
  The example is meant to be booted on its own. 0xAAAA_AAAA sits 682 MiB above
  the start of DDR, so that much memory has to be populated.
* The SBL is built and the board is set to UART boot mode. See
  \ref UART_BOOTLOADER_PYTHON_SCRIPT and \ref BOOTMODE_UART.

# Execution Flow

The application does the following:

* Initializes the board and the drivers, enables the VPAC0 module clock and
  initializes the UDMA driver instance of BCDMA_0 with `enableUtc` set.
* Reads `DRU_CAPABILITIES` and stops if the TR type it is about to use is not
  advertised by the instance.
* Programs DRU queue 0 with a priority, QoS, order ID, consecutive transaction
  count and re-arbitration wait, then reads `DRU_QUEUE CFG` back and compares it
  against the value that was written.
* Opens the UTC channel with ring memory, configures it, moves the owner bit,
  confirms that `CHNRT CHST_SCHED` really points at the queue that was asked
  for, and enables the channel.
* Builds the reference payload, computes its CRC-32 and writes payload and
  signature into the buffer at 0x9555_5555.
* Runs the four transfers. For each one it clears the destination, lays down the
  guard pattern, writes the caches back, builds and submits the TRPD, waits for
  the response, invalidates the caches and then runs the three checks on what
  arrived.
* Disables and closes the channel and de-initializes the UDMA driver.

The transfers and the three checks that follow each of them are the diagnostic.
The capability check and the configuration readbacks are not part of it; they
are there so that a setup problem is reported as a setup problem instead of
turning up later as a data mismatch.

All the test buffers are cacheable, so the cache maintenance is not optional.
The source and the destination are written back before the transfer is
submitted, and the destination is invalidated after the response arrives and
before the CPU reads it.

\code
CacheP_wb(destBuf, APP_BUF_SIZE, CacheP_TYPE_ALLD);
/* transfer */
CacheP_inv(destBuf, APP_BUF_SIZE, CacheP_TYPE_ALLD);
\endcode

## Checks Run After Every Transfer

 Check   | What it proves | On failure
 --------|----------------|------------
 Guard   | The DRU wrote nothing past `destination + 128` | Offset and the value found
 Payload | The bytes that arrived are the bytes that were sent | First differing offset
 CRC     | The payload and its embedded signature still agree | Embedded and recomputed values

A failing check stops the transfer it belongs to, and the remaining transfers
are skipped, so the log ends at the first problem instead of repeating it four
times. If the transfer itself fails, the state of the DRU channel is dumped:
`CHRT_CTL`, `CHRT_STATUS_DET`, `RING_FWD_OCC`, `RING_RCVS_OCC` and the `CAUSE`
register holding the slot of the channel.

# Fault-Injection Scenarios

In normal operation this diagnostic passes, and there is no way to make the DRU
mis-transfer a buffer from the outside. Failures have to be injected into the
code. The edits below exercise one check each.

**Payload check.** Corrupt a payload byte before the cache writeback in
`App_initSrcBuf()`, so that the corruption has reached memory by the time the
DRU reads it:

\code
srcBuf[10U] ^= 0xFFU;
\endcode

\code
Running tests for Guard .. OK
Running tests for Payload ..
[Error] Payload mismatch at offset 10, expected 0x0a got 0xf5!!
\endcode

**CRC check.** Corrupt one of the four signature bytes in the same place
instead. The payload arrives intact, so only the signature comparison trips:

\code
srcBuf[APP_PAYLOAD_SIZE] ^= 0xFFU;
\endcode

\code
Running tests for Payload .. OK
Running tests for CRC ..
[Error] CRC check failed, embedded 0x545a743f calculated 0x545a74c0!!
\endcode

**Guard check.** Ask for more bytes than the transfer holds, in
`App_transferTest()`:

\code
retVal = AppDru_memcpy(chHandle, pDruRegs, chNum, step->destAddr,
                       step->srcAddr, APP_XFER_SIZE + 8U);
\endcode

The eight extra bytes are read from the tail of the source and land in the guard
region of the destination:

\code
Running tests for Guard ..
[Error] Overrun detected at offset 128, expected 0x5a got 0xa5!!
\endcode

**Unsupported TR type.** Setting `APP_TR_TYPE` to `UDMA_TR_TYPE_4` is caught by
the capability check before any transfer is attempted. Removing that check as
well lets the TR reach the DRU, which returns an unsupported TR response, and
the state of the channel is dumped.

# Build Steps

Build using the makefile target for the combination (see
\ref MAKEFILE_BUILD_PAGE), or import the CCS project instead (see
\ref CCS_PROJECTS_PAGE). From the SDK root folder,

\code
make -s -f makefile.j722s dru_basic_tests_with_crc_check_j722s-evm_main-r5fss0-0_nortos_ti-arm-clang
\endcode

The application image `dru_basic_tests_with_crc_check.release.appimage.hs_fs` is
generated in
`examples/ip_fma/dru_basic_tests_with_crc_check/j722s-evm/main-r5fss0-0_nortos/ti-arm-clang`.

# Steps to Run the Example

* Point the appimage path in
  `tools/boot/sbl_prebuilt/@VAR_BOARD_NAME_LOWER/default_sbl_uart_hs_fs.cfg` to
  the image that was built.
* Send the SBL and the appimage over UART as described in
  \ref UART_BOOTLOADER_PYTHON_SCRIPT.
* Connect to the UART terminal within 5 seconds of the script finishing to see
  the application logs.

# Sample Output

Shown below is a sample output when the application is run,

\code
DRU basic tests with CRC check
Initial CRC calculation based on the payload data
... CRC value: 0x545a74c0

DMA transfer of buffer from address 0x95555555 to address 0xaaaaaaaa
Running tests for Guard .. OK
Running tests for Payload .. OK
Running tests for CRC .. OK

DMA transfer of buffer from address 0xaaaaaaaa to address 0x95555555
Running tests for Guard .. OK
Running tests for Payload .. OK
Running tests for CRC .. OK

DMA transfer of buffer from address 0x95555555 to address 0x79102a00
Running tests for Guard .. OK
Running tests for Payload .. OK
Running tests for CRC .. OK

DMA transfer of buffer from address 0x79102a00 to address 0x95555555
Running tests for Guard .. OK
Running tests for Payload .. OK
Running tests for CRC .. OK
All tests have passed!!
\endcode

The configuration steps print nothing when they succeed, so a passing run always
looks exactly like the above.

# Limitations and Notes

* The VPAC0 DRU implements three scheduling queues, 0 to 2, and a channel on
  this instance has to be scheduled on one of them. `CSL_DRU_NUM_QUEUE` is the
  queue count of the SoC level DRU of other devices and does not apply here.
  The example uses queue 0.
* This DRU supports TR types 8 to 11, so a TR built for this instance has to use
  one of those types.
* The buffer addresses are compile time constants. Running the example in a
  system where another core or Linux owns part of DDR requires picking addresses
  outside those carve-outs, and a change to 0x9555_5555 or 0xAAAA_AAAA has to
  keep the address line coverage described in
  \ref IP_FMA_DRU_BASIC_TESTS_BUFFERS intact.
* The DDR addresses are not cache line aligned. Cache maintenance rounds down to
  the start of the line, so nothing else should be placed in the lines the test
  buffers share.

# See Also

\ref IP_FMA_PAGE
