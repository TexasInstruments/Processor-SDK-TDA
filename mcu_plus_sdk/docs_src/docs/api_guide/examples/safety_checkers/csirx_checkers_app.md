# CSIRX SAFETY CHECKERS APP {#EXAMPLES_CSIRX_CHECKERS_APP}

[TOC]

# Introduction

CSIRX Safety Checker (SAFETY_CHECKERS-CSIRX) provided APIs which are integrated into the safety application to verify the CSIRX module configuration and validate the runtime CSIRX configuration against golden reference.

SAFETY_CHECKERS-CSIRX library includes the following implementation:
1. Reading the configuration registers of CSIRX modules.
2. Validation of current CSIRX configuration registers against golden reference.
3. Validation of VIM, Sensor, and QoS configurations.
4. Verification of bandwidth limits for CSIRX IP.

CSIRX safety checkers application demonstrates the usage of all the features provided by the CSIRX safety checkers library.

Following are the test cases provided in this application:

* Get & verify register configuration APIs with possible input parameters.
* Verify available CSIRX bandwidth with provided framesize and fps.
* Negative test cases with NULL parameters to verify error handling.

In generation of CSIRX config, Safety application shall call the CSIRX read APIs to access the configuration registers from the Safety Checkers library running in the Safety Core. Safety checkers returns CSIRX config to the safety application. Safety application validates the CSIRX configuration and save it as a golden reference in a non-volatile memory. This initializes the Safety Application and gathers the initial configuration data.

In validate CSIRX config stage, Safety application provides golden reference to the CSIRX safety checker and Safety checker will validates at defined intervals. CSIRX safety checker reads the CSIRX registers at runtime and validate they are matching with the golden state for modules in safety loop. Safety checker will return success or failure after validates against the golden reference.

The user should implement firewall based protection for golden reference and also create a checksum for the golden reference to ensure validity of the golden reference data.

# Supported Combinations {#EXAMPLES_CSIRX_CHECKERS_APP_COMBOS}

\cond SOC_J722S

 Parameter      | Value
 ---------------|-----------
 CPU + OS       | main-r5fss0-0 freertos
 Toolchain      | ti-arm-clang
 Board          | @VAR_BOARD_NAME_LOWER
 Example folder | source/safety_checkers/examples/

\endcond

# Steps to Run the Example

- **When using CCS projects to build**, import the CCS project for the required combination
  and build it using the CCS project menu (see \ref CCS_PROJECTS_PAGE).
- **When using makefiles to build**, note the required combination and build using
  make command (see \ref MAKEFILE_BUILD_PAGE)
- Launch a CCS debug session and run the executable, see \ref CCS_LAUNCH_PAGE

# See Also

\ref CSIRX_SAFETY_CHECKERS_PAGE

# Sample Output

Shown below is a sample output when the application is run,

\code
SAFETY_CHECKERS_CSIRX_APP: CSIRX safety checkers Application started
SAFETY_CHECKERS_CSIRX_APP: Verification of safety checkers is successful
SAFETY_CHECKERS_CSIRX_APP: Number of frames recieved is 302
SAFETY_CHECKERS_CSIRX_APP: Sample Application - DONE !!!
\endcode
