# CSIRX_SAFETY_CHECKERS {#CSIRX_SAFETY_CHECKERS_PAGE}

## Introduction

The CSIRX Safety Checker is a comprehensive safety library designed to ensure the integrity and reliability of the Camera Serial Interface Receiver (CSIRX) module. It provides productized APIs for detecting failure modes and configuration anomalies in the CSIRX module through continuous register monitoring and validation.

The CSIRX module is critical for camera sensor data acquisition in safety-critical embedded systems. The Safety Checker implements a two-phase verification strategy:

**Phase 1: Golden Reference Generation** - Captures and stores initial CSIRX register configurations during system initialization as a baseline reference for future validations.

**Phase 2: Runtime Verification** - Continuously monitors CSIRX register values at runtime, comparing current configurations against the golden reference to detect any unintended changes or deviations.

The library monitors and validates the following CSIRX components:
- Stream Control Registers for data stream configuration
- Virtual Channel Configuration for multi-stream data handling
- DPHY Configuration, PLL, and Lane settings for receiver configuration
- Data Type and Framesize Configuration for frame dimension settings
- VIM (Vector Interrupt Manager) configuration for CSIRX event handling
- Quality of Service (QoS) settings via UDMA channel configuration
- Sensor configuration accessed through I2C interface

The CSIRX Safety Checker library provides comprehensive failure detection capabilities including APIs to read and store CSIRX register configurations as golden reference, verify runtime configurations against the golden reference, validate VIM and Sensor configurations, verify QoS settings, ensure framesize and fps parameters remain within CSIRX bandwidth limits, support multiple CSIRX instances, and provide detailed error reporting for configuration anomalies.

## Features Supported

The module supports below API's for the application

* API to get register configuration of CSIRX module registers (Stream control, DPHY config, DPHY PLL, DPHY Lane config, Virtual Channel, Data type & Framesize).
* API to read CSIRX module registers at runtime and validate they match the golden state for modules in the safety loop.
* API to get VIM register configuration for CSIRX events.
* API to verify VIM register configuration against golden reference.
* API to get sensor configuration via I2C interface.
* API to verify sensor configuration against golden reference.
* API to get Quality of Service (QoS) settings for CSIRX.
* API to verify QoS settings against golden reference.
* API to verify if the provided configuration of framesize and fps is within the bandwidth limits of CSIRX.

## SysConfig Features

- None

## Features NOT Supported

- None

## Important Usage Guidelines

Safety application shall consider the following recommendations for robust operation of the safety checker APIs:

* Memory allocated for golden register reference shall be firewall protected
* Implement appropriate MPU configurations to isolate safety application memory from other applications
* Implement a watchdog timeout for safety checker API handling
* Golden reference should be generated offline and saved as a benchmark for subsequent validations
* The read and save configuration step should be done only once, upon freezing the safety loop
* Create a checksum for the golden reference to ensure validity of the golden reference data

## Example Usage

The following shows an example of CSIRX Safety Checkers API usage

### Include Files

Include the below file to access the APIs

\code{.c}
#include "safety_checkers_csirx.h"
\endcode

### Step 1: Get CSIRX Register Configuration

Uses the pointer to register configuration as input and updates the array with the register dump of the CSIRX registers specified.

\code{.c}
status = SafetyCheckers_csirxGetRegCfg(regCfg,
                                       SAFETY_CHECKERS_CSIRX_REG_TYPE_STRM_CTRL,
                                       instance);
\endcode

Here, regType is SAFETY_CHECKERS_CSIRX_REG_TYPE_STRM_CTRL. Hence stream control register configuration is read and populated in regCfg.

### Step 2: Save as Golden Reference

After reading the register configurations, save the configuration data as the golden reference. This serves as the benchmark for subsequent validations. The saved reference should be stored in a header file and used in the safety loop.

\code{.c}
/* Save regCfg as golden reference */
/* Create header file with golden_regCfg array */
/* Rebuild application with golden reference */
\endcode

### Step 3: Verify CSIRX Register Configuration

Compare the golden reference with runtime CSIRX register values and return success or failure.

\code{.c}
status = SafetyCheckers_csirxVerifyRegCfg(regCfg,
                                          SAFETY_CHECKERS_CSIRX_REG_TYPE_STRM_CTRL,
                                          instance);

if (status == SAFETY_CHECKERS_REG_DATA_MISMATCH)
{
    DebugP_log("CSIRX register mismatch with Golden Reference!!\r\n");
}
else if (status == SAFETY_CHECKERS_SOK)
{
    DebugP_log("No CSIRX register mismatch with Golden Reference\r\n");
}
\endcode

### Step 4: Verify VIM Configuration

Get and verify VIM register configuration for CSIRX events.

\code{.c}
status = SafetyCheckers_csirxGetVimCfg(drvHandle, &vimCfg);

if (status == SAFETY_CHECKERS_SOK)
{
    status = SafetyCheckers_csirxVerifyVimCfg(drvHandle, &vimCfg);
    
    if (status == SAFETY_CHECKERS_REG_DATA_MISMATCH)
    {
        DebugP_log("VIM configuration mismatch!!\r\n");
    }
}
\endcode

### Step 5: Verify Sensor Configuration

Get and verify sensor configuration via I2C interface.

\code{.c}
status = SafetyCheckers_csirxGetSensorCfg(i2cHandle, slaveAddr, regData);

if (status == SAFETY_CHECKERS_SOK)
{
    status = SafetyCheckers_csirxVerifySensorCfg(i2cHandle, slaveAddr, regData);
    
    if (status == SAFETY_CHECKERS_REG_DATA_MISMATCH)
    {
        DebugP_log("Sensor configuration mismatch!!\r\n");
    }
}
\endcode

### Step 6: Verify QoS Configuration

Get and verify Quality of Service settings for CSIRX.

\code{.c}
status = SafetyCheckers_csirxGetQoSCfg(&qosSettings, drvHandle);

if (status == SAFETY_CHECKERS_SOK)
{
    status = SafetyCheckers_csirxVerifyQoSCfg(&qosSettings, drvHandle);
    
    if (status == SAFETY_CHECKERS_REG_DATA_MISMATCH)
    {
        DebugP_log("QoS configuration mismatch!!\r\n");
    }
}
\endcode

### Step 7: Verify Bandwidth Limits

Verify if the provided configuration of framesize and fps is within the bandwidth limits of CSIRX.

\code{.c}
status = SafetyCheckers_csirxVerifyCsiAvailBandwidth(channel, 30);

if (status == SAFETY_CHECKERS_SOK)
{
    DebugP_log("Bandwidth verification passed - 30 fps is achievable\r\n");
}
else
{
    DebugP_log("Bandwidth verification failed - 30 fps exceeds CSIRX bandwidth limits\r\n");
}
\endcode

## Register Configuration Types

The following register configuration types are supported:

* **SAFETY_CHECKERS_CSIRX_REG_TYPE_STRM_CTRL** - Stream control registers
* **SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_CONFIG** - DPHY configuration registers
* **SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_PLL** - DPHY PLL registers
* **SAFETY_CHECKERS_CSIRX_REG_TYPE_DPHY_LANE_CONFIG** - DPHY Lane configuration registers
* **SAFETY_CHECKERS_CSIRX_REG_TYPE_VIRTUAL_CHANNEL** - Virtual Channel configuration registers
* **SAFETY_CHECKERS_CSIRX_REG_TYPE_DATATYPE_FRAMESIZE** - Data type & Framesize configuration registers

## Design Workflow

The CSIRX Safety Checker follows a two-phase workflow:

### Phase 1: Golden Reference Generation (One-time, Offline)

1. Initialize the safety application
2. Call register read APIs to collect current register configuration
3. Save the collected data as the golden reference
4. Store the golden reference in memory protected by firewall/MPU
5. Create checksum for golden reference validation

### Phase 2: Runtime Verification (Continuous, in Safety Loop)

1. Call register verify APIs at regular intervals
2. Compare current register values against the golden reference
3. Detect any deviations from the golden reference
4. Return error status if mismatches are detected
5. Continue monitoring for any configuration changes

## API

\ref CSIRX_SAFETY_CHECKERS
