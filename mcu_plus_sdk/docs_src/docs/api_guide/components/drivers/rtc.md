# RTC {#DRIVERS_RTC_PAGE}

[TOC]

The basic purpose for the RTC is to keep time of day. The other equally important purpose of RTC is for Digital
Rights management. Some degree of tamper proofing is needed to ensure that simply stopping, resetting, or
corrupting the RTC does not go unnoticed so that if this occurs, the application can re-acquire the time of day
from a trusted source.

## Features Supported

- Supported 15 bit 32768Hz counter.
- Supported 48 bit seconds counter with +/- 1 30uS upto +/- 1 S drift adjustment every 4048 Seconds.
- Support for 256 bits of Scratch PAD
- 1 ON_OFF compare event, 48-bits
- 1 OFF_ON compare event, 48-bits
- 2 event outputs OFF_ON and ON_OFF to CORE
- Functional lockout, unlock by special vbusp sequence
- HOST PROCESSOR can update MMRs without polling, thanks to HW Shadow/Auto Sync
- HOST PROCESSOR can read time without polling, thanks to HW Shadow/Auto Sync

## SysConfig Features

@VAR_SYSCFG_USAGE_NOTE

- RTC instance name
- Input clock frequency to be used for RTC module
- Enable Freeze Mode for RTC.
- Enable Interrupt and Assign User Callback
- Support to attach callback functions to each of the ON_OFF/OFF_ON events

## Features NOT Supported

- PMIC enable support

## Example Usage

Include the below file to access the APIs
\snippet Rtc_sample.c include

Instance Open Example
\snippet Rtc_sample.c open

Instance Close Example
\snippet Rtc_sample.c close

## API

\ref DRV_RTC_MODULE
