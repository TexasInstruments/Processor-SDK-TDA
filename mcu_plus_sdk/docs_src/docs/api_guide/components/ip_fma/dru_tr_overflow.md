# DRU – UTC Active TR event overflow detection check {#IP_FMA_DRU_TR_OVERFLOW_PAGE}

[TOC]

# Introduction

The Data Routing Unit (DRU) is the DMA engine that processes the Transfer
Requests (TR) submitted on a UTC channel. Every DRU instance keeps a set of
CAUSE registers that record the error conditions of its channels, one of them
being the overflow of an active TR event.

A channel accepts one trigger at a time. If a second trigger arrives while the
first one has not been consumed by a TR, the trigger cannot be queued and the
DRU flags an overflow for that channel. This example provokes that condition on
purpose and checks that the DRU reports it in the CAUSE register of the channel
under test.

The example is located at
`${SDK_INSTALL_PATH}/examples/ip_fma/dru_active_tr_event_overflow_detection`.
It uses the DRU of the VPAC0 module, which is reachable from the main R5F and is
exposed by the UDMA driver as a UTC instance of BCDMA_0.

# Supported Combinations {#IP_FMA_DRU_TR_OVERFLOW_COMBOS}

\cond SOC_J722S

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | main-r5fss0-0 nortos
 Toolchain      | ti-arm-clang
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | examples/ip_fma/dru_active_tr_event_overflow_detection

\endcond

# Overflow Reporting in the CAUSE Registers

The DRU has four CAUSE registers, each of them 64 bits wide and covering 16
channels. A channel occupies a 4 bit slot inside its register, and the active TR
event overflow is reported by bit 1 of that slot. The register index and the bit
position of a channel are derived from the channel number returned by the UDMA
driver.

\code
causeIdx = chNum / APP_DRU_NUM_CH_IN_CAUSE_REG;
bitBase  = (chNum % APP_DRU_NUM_CH_IN_CAUSE_REG) * APP_DRU_CAUSE_CH_SLOT_WIDTH;
mask     = APP_DRU_CAUSE_CH_OVERFLOW_MASK << bitBase;

if((CSL_REG64_RD(&pDruRegs->CAUSE.CAUSE.CAUSE[causeIdx]) & mask) != 0ULL)
{
    DebugP_log("UDMA overflow detected on channel %u\r\n", chNum);
}
\endcode

# Preconditions and Assumptions

* The board is a @VAR_BOARD_NAME.
* The VPAC0 module clock is enabled before the DRU registers are accessed. The
  example does this with `SOC_moduleClockEnable()`.
* UTC support is enabled in the UDMA init parameters, otherwise the DRU channels
  of BCDMA_0 cannot be opened.

# Execution Flow

The application performs the following steps:

* Initializes the board and the drivers, enables the VPAC0 module clock and
  initializes the UDMA driver instance of BCDMA_0 with `enableUtc` set.
* Opens a UTC channel on the VPAC0 DRU, configures it as directly controlled
  (`CSL_DRU_OWNER_DIRECT_TR`) on DRU queue 3 and enables it.
* Reads all four CAUSE registers and checks that they are zero. A non zero value
  at this point means that an error condition is already pending, and the test
  is stopped.
* Issues two consecutive software triggers on the enabled channel without
  submitting any TR that would consume the first one.
* Reads the CAUSE register holding the slot of the channel and checks the
  overflow bit of that channel. The test passes only if the bit is set.
* Disables and closes the channel and de-initializes the UDMA driver.

# Fault-Injection Scenario

The fault is injected by the two `Udma_chSetSwTrigger()` calls on the same active
channel. Retriggering a channel whose event has not been consumed is an invalid
sequence, and the DRU reacts to it by raising the overflow bit for that channel.

\code
retVal = Udma_chSetSwTrigger(chHandle, trigger);
retVal = Udma_chSetSwTrigger(chHandle, trigger);
\endcode

# Steps to Run the Example

- **When using CCS projects to build**, import the CCS project for the required
  combination and build it using the CCS project menu (see \ref CCS_PROJECTS_PAGE).
- **When using makefiles to build**, note the required combination and build using
  make command (see \ref MAKEFILE_BUILD_PAGE). From the SDK root folder,
  \code
  make -s -f makefile.j722s dru_tr_of_j722s-evm_main-r5fss0-0_nortos_ti-arm-clang
  \endcode
- Flash SBL NULL bootloader by following steps mentioned in \ref EVM_FLASH_SOC_INIT
- Switch to \ref BOOTMODE_OSPI and power on the EVM.
- Launch a CCS debug session and run the executable, see \ref CCS_LAUNCH_PAGE

# Sample Output

Shown below is a sample output when the application is run,

\code
DRU Active TR Event Overflow Detection
Udma channel number 0
Test core execution begin
Before swTrig writing check, all CAUSE registers are ZERO - Check passed
UDMA overflow detected on channel 0
  CAUSE[0] = 0x0000000000000002
All tests have passed!!
\endcode

The channel number depends on the channel allocated by the UDMA driver, so the
reported CAUSE register index and value can differ from the ones shown above.

# Limitations and Notes

* Only UTC channels are covered, since the CAUSE registers belong to the DRU.
* The example checks the DRU of VPAC0. The local DRUs of the C7x subsystems
  cannot be reached from the main R5F.
* The overflow bits are not cleared by the example. A second run on the same
  channel without a reset fails the pre-check that expects all CAUSE registers
  to be zero.

# See Also

\ref IP_FMA_PAGE
