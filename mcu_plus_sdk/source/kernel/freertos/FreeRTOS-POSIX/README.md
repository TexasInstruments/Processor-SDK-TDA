## FreeRTOS+POSIX: Portable Operating System Interface (POSIX threading wrapper) for FreeRTOS

The Portable Operating System Interface (POSIX) is a family of standards specified by the IEEE Computer Society for maintaining compatibility between operating systems. FreeRTOS+POSIX implements *a small subset* of the [POSIX threading](https://pubs.opengroup.org/onlinepubs/7908799/xsh/threads.html) API. This subset allows application developers familiar with POSIX API to develop a FreeRTOS application using POSIX like threading primitives. FreeRTOS+POSIX does not implement more than 80% of the POSIX API. Therefore, an existing POSIX compliant application or a POSIX compliant library cannot be ported to run on FreeRTOS Kernel using only this wrapper. 


This repository only contains source code. For demo applications, please visit https://github.com/FreeRTOS/FreeRTOS-Labs. 

## Usage in MCU+ SDK

This fork of FreeRTOS+POSIX repository is consumed by MCU+ SDK as a submodule.
It is used for building the FreeRTOS Kernel Libraries for different cores (R5,
M4, A53, C66). This enables one to use POSIX APIs in the application using the
FreeRTOS Kernel. Please refer [MCU+ SDK Git Repository](https://github.com/TexasInstruments/mcupsdk-core)
for documentation and instructions to setup the build.
