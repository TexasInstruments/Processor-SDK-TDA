# DMA Utils autoincrement 1d2d3d example {#EXAMPLES_DMAUTILS_AUTOINC_1D2D3D}

[TOC]

# Introduction

Demostrates a simple application demonstrating 1D, 2D, 3D auto increment feature of dmautils

The application runs on C7x core. UDMA is configured for UTC/DRU mode.

# Supported Combinations

\cond SOC_AM62AX

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | c75ss0-0 freertos
 Toolchain      | ti-arm-clang
 Boards         | @VAR_BOARD_NAME_LOWER
 Example folder | examples/dmautils/dmautils_autoinc_1d2d3d/

\endcond

\cond SOC_AM62DX

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | c75ss0-0 freertos
 Toolchain      | ti-arm-clang
 Boards         | @VAR_BOARD_NAME_LOWER
 Example folder | examples/dmautils/dmautils_autoinc_1d2d3d/

\endcond

\cond SOC_AM275X

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | c75ss0-0 freertos
 ^              | c75ss1-0 freertos
 Toolchain      | ti-c7000
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | examples/dmautils/dmautils_autoinc_1d2d3d/

\endcond

# Steps to Run the Example

- **When using CCS projects to build**, import the CCS project for the required combination
  and build it using the CCS project menu (see \ref CCS_PROJECTS_PAGE).
- **When using makefiles to build**, note the required combination and build using
  make command (see \ref MAKEFILE_BUILD_PAGE)

- Launch a CCS debug session and run the executable, see \ref CCS_LAUNCH_PAGE


# Sample Output

Shown below is a sample output when the  application is run,

\code
Image width : 40
Image height : 16
DMAUtils TestCase 0,        PASSED
Image width : 40
Image height : 16
DMAUtils TestCase 1,        PASSED
Image width : 40
Image height : 16
DMAUtils TestCase 2,        PASSED
All tests have passed!!
\endcode
