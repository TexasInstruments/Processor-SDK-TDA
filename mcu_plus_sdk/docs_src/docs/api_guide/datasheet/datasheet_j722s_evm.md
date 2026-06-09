# Datasheet {#DATASHEET_J722S_EVM}

[TOC]

## Introduction

This datasheet provides the performance numbers of various device drivers in MCU PLUS SDK for @VAR_SOC_NAME


## Generic Setup details

SOC Details             | Values
------------------------|------------------------------
Core                    | R5F
Core Operating Speed    | 800 MHz
Cache Status            | Enabled

Optimization Details    | Values
------------------------|------------------------------
Build Profile           | Release
R5F Compiler flags      | -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mthumb -Wall -Werror -g -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function
R5F Linker flags        | -Wl,--diag_suppress=10063 -Wl,--priority -Wl,--ram_model -Wl,--reread_libs
Code Placement          | HSM RAM (For SBL), DDR (others)
Data Placement          | HSM RAM (For SBL), DDR (others)

## Performance Numbers

### Early CAN Response

CAN response is measured from MCU_PORz Reset to pulling the CAN-H line out of standby.

Device           | Measured Time (ms)
-----------------|----------------------------------------
J722S HS-FS      | 52.15

### IPC performance

#### IPC NOTIFY

- 10000 messages are sent and average one way message latency is measured

Local Core  | Remote Core | Average Message Latency (us)
------------|-------------|------------------------------
wkup-r5f0-0 | mcu-r5f0-0  |  1.35
wkup-r5f0-0 | main-r5f0-0 |  1.08
wkup-r5f0-0 | c75ss0      |  1.21
wkup-r5f0-0 | c75ss1      |  1.46

#### IPC RPMSG

- 10000 messages are sent and average one way message latency is measured

Local Core  | Remote Core | Message Size | Average Message Latency (us)
------------|-------------|--------------|------------------------------
wkup-r5f0-0 | mcu-r5f0-0  | 4            | 9.639
wkup-r5f0-0 | mcu-r5f0-0  | 32           | 15.927
wkup-r5f0-0 | mcu-r5f0-0  | 64           | 22.842
wkup-r5f0-0 | mcu-r5f0-0  | 112          | 33.113
wkup-r5f0-0 | main-r5f0-0 | 4            | 9.172
wkup-r5f0-0 | main-r5f0-0 | 32           | 14.513
wkup-r5f0-0 | main-r5f0-0 | 64           | 20.434
wkup-r5f0-0 | main-r5f0-0 | 112          | 29.322
wkup-r5f0-0 | c75ss0      | 4            | 13.463
wkup-r5f0-0 | c75ss0      | 32           | 17.124
wkup-r5f0-0 | c75ss0      | 64           | 20.174
wkup-r5f0-0 | c75ss0      | 112          | 24.606
wkup-r5f0-0 | c75ss1      | 4            | 14.442
wkup-r5f0-0 | c75ss1      | 32           | 17.527
wkup-r5f0-0 | c75ss1      | 64           | 20.471
wkup-r5f0-0 | c75ss1      | 112          | 24.959
