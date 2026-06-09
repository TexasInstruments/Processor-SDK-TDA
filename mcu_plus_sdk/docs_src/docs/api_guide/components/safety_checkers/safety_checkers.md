# Safety Checkers Library {#SAFETY_CHECKERS_PAGE}

[TOC]

\attention Safety Checker Library should be used with a Safety OS.The freeRTOS examples provided are only for reference.

# Introduction

The @VAR_SOC_NAME family of SoCs provides various safety mechanisms and features, as well as recommendations for usage of these safety mechanisms and features. The Safety Checkers Library provides productized APIs for ensuring the safety of TIFS, RM , PM and CSIRX modules. SC provides these interfaces to assist in the development of software applications involving Functional Safety.

The high-level design of the Safety checker implementation for the TIFS, RM , PM and CSIRX module. The primary objective of implementing the safety checker is to make sure the detection of the failure modes for TIFS, RM, PM and CSIRX module are handled properly. Also providing the safety examples as a reference which calls the checker APIs to verify the module operations are done properly during the boot.

\imageStyle{Safety_Checkers.png,width:50%}
\image html Safety_Checkers.png "Safety Checkers Diagram"

SC consists of below sub-modules

\cond SOC_AM62X || SOC_AM62AX || SOC_AM62PX || SOC_AM62DX || SOC_J722S
- \subpage PM_SAFETY_CHECKERS_PAGE
- \subpage RM_SAFETY_CHECKERS_PAGE
- \subpage TIFS_SAFETY_CHECKERS_PAGE
\endcond
\cond SOC_J722S
- \subpage CSIRX_SAFETY_CHECKERS_PAGE

The CSIRX Safety Checkers (Detailed Documentation) page provides comprehensive information about:
- CSIRX module safety checker design and principles
- Configuration register monitoring
- Golden reference generation and verification procedures
- Detailed API reference for all CSIRX safety checker functions
- Example usage and implementation guidelines
\endcond

Following example demonstrates the usage of RM, PM and TIFS safety checkers.
\cond SOC_AM62X || SOC_AM62AX || SOC_AM62PX || SOC_AM62DX || SOC_J722S
- \subpage SAFETY_APP_PAGE
\endcond
