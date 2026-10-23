# IP FMA {#IP_FMA_PAGE}
​
[TOC]
​
# Introduction
​
This user guide provides an overview and usage information for the safety
mechanisms implemented within IP FMA (Field Monitoring Application).
​
The IP FMA safety mechanism documentation covers the following:
​
* Overview of each mechanism
* Preconditions and assumptions
* Build steps and integration details
* Execution flow
* Limitations and notes
* Fault-injection scenarios (when applicable)
​
The source code of IP FMA is located under
`$(MCU_PLUS_SDK_PATH)/source/ip_fma` directory.
​
# List of Examples for Safety Mechanisms
​
The following examples for safety mechanisms are supported and documented
in this guide. Select a mechanism below to navigate to its dedicated user
guide.
​
* \subpage IP_FMA_DRU_REGS_PERIODIC_READBACK_PAGE - DRU – Periodic Software Readback of Static
  Configuration Registers
* \subpage IP_FMA_DRU_TR_OVERFLOW_PAGE - DRU – UTC Active TR event overflow detection check
* \subpage IP_FMA_DRU_BASIC_TESTS_WITH_CRC_CHECK - DRU – Basic Tests with CRC Check
* \subpage IP_FMA_CSIRX_PAGE - CSI RX – Periodic Software Readback of Static
  Configuration Registers
* \subpage IP_FMA_CLK_PAGE - CLK – Periodic Software Read-back of Static
  Configuration Registers
* \subpage SDL_DCC_RB_WRITTEN_CONFIG_PAGE - CLK - Software Readback of Written Configuration
  Test
* \subpage IP_FMA_UTC_LEGAL_TR_CHECK_PAGE - UTC Legal TR Check - DRU unsupported/illegal TR detection