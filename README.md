# TI Processor SDK

> A multi-core software development platform for TI Jacinto™ processors, designed for RTOS+Linux/QNX based development across ADAS, vision, robotics, gateway, and industrial use cases.


## Overview

TI Processor SDK for Jacinto™ provides a software foundation for developing applications on Jacinto family processors such as TDA4VM, TDA4AH, and related J7 devices. TI describes the SDK as a multi-processor platform that combines Processor SDK Linux or Processor SDK QNX with Processor SDK RTOS to enable development across heterogeneous cores and hardware accelerators.


## Key Features

- Multi-processor architecture support across Arm, DSP, and MCU domains.
- Linux, QNX, and RTOS-based development flows, depending on the selected SDK package.
- Support for vision, deep learning, robotics, ADAS, and gateway applications.
- Access to hardware-accelerated software blocks such as TIDL, OpenVX, vision preprocessing/postprocessing, and peripheral drivers through the RTOS companion package.
- Integration with TI training, documentation, and example applications for faster bring-up.


## Supported Devices and Platforms

TI’s Processor SDK for Jacinto™ family includes support for devices such as:
- J722S (TDA4VEN and TDA4AEN)
- J721E (DRA829 and TDA4VM)
- J721S2 (TDA4VL-Q1 and TDA4AL-Q1)
- J742S2 (TDA4APE-Q1 and TDA4VPE-Q1)
- J784S4 (TDA4AP-Q1, TDA4VP-Q1, TDA4AH-Q1 and TDA4VH-Q1)


## Typical Use Cases

This SDK is commonly used for:

- ADAS applications such as surround-view, perception, and sensor fusion.
- Robotics workloads using ROS-based stacks and edge AI pipelines.
- Vision processing and image analytics.
- Industrial and gateway applications requiring real-time control and networking.
- Deep-learning inference using hardware acceleration on heterogeneous compute cores.


## Software Architecture

TI’s Processor SDK model is built around heterogeneous execution. The Arm® Cortex®-A72/A53 typically runs Linux or QNX, while RTOS workloads run on Arm® Cortex®-R5F, the C7x/C66 DSP, or hardware accelerators depending on the SoC and use case. TI’s training materials emphasize that U-Boot is used in the boot flow, Linux runs on the Arm® application domain, and TI provides RTOS boot and firmware support for early-stage and real-time components.

In practical terms, the architecture lets you split the application into:
- High-level application logic on Linux/QNX.
- Real-time tasks and drivers on RTOS.
- Vision and AI acceleration through hardware-optimized libraries and frameworks.


## Getting Started

### Prerequisites
- A supported TI Jacinto™ EVM.
- A Linux host machine for building and flashing the SDK components (Ubuntu 22.04 recommended).

### 1. Set up the host environment

Install the following packages on the Linux Host Machine:
```bash
sudo apt update
sudo apt install -y build-essential git git-lfs python3 python3-pip wget unzip lib32ncurses6 lib32z1 libyaml-cpp-dev
```

### 2. Clone and set up the SDK dependencies

- Clone the repository:
```bash
git clone https://github.com/TexasInstruments/Processor-SDK-TDA.git
```
- Navigate to the project directory:
```bash
cd Processor-SDK-TDA
```
- Export the required environment variables:
```bash
export SOC=<soc_name>  # Options: j722s, j721e, j721s2, j742s2, j784s4
export TISDK_IMAGE=adas
```
- Run the setup script to download and prepare the necessary files:
```bash
./setup.sh
```

### 3. Build the SDK components

- Build SDK components using the sdk_builder:
```bash
cd sdk_builder
make sdk -j $(nproc)
# or 
./make_sdk.sh
```

### 4. Flash and run on target

Once images are built, deploy them to the supported EVM or target hardware and validate boot, connectivity, display, vision, and accelerator demos. TI’s examples and training material are designed around EVM bring-up and out-of-box testing.

- Create a bootable SD card:
```bash
# run the following command from the project root (Processor-SDK-TDA) directory
sudo ./sdk_builder/scripts/mk-linux-card.sh /dev/<sdX>  # replace <sdX> with your SD card device
```

- Copy the Linux rootfs/bootfs to the SD card:
```bash
# mount the SD card partitions before running the following command
./sdk_builder/scripts/install_to_sd_card.sh
```

- Install the firmware and RTOS components to the SD card:
```bash
cd sdk_builder
make linux_fs_install_sd
```

### 5. Download and install data set and TIDL models

- Download the TIDL models and data sets for testing
```bash
# Common data set 
wget https://github.com/TexasInstruments/Processor-SDK-TDA/releases/download/CICD_12_00_00_00/psdk_rtos_ti_data_set_12_00_00_00.tar.gz

# TIDL models (SoC-specific) 
# Example for J784S4
wget https://github.com/TexasInstruments/Processor-SDK-TDA/releases/download/CICD_12_00_00_00/psdk_rtos_ti_tidl_models_12_00_00_00_j784s4.tar.gz
```

- Install the data sets and models to the target:
```bash
# install common data set
./sdk_builder/scripts/install_data_set_to_sd_card.sh psdk_rtos_ti_data_set_12_00_00_00.tar.gz

# install TIDL models (example for J784S4)
./sdk_builder/scripts/install_data_set_to_sd_card.sh psdk_rtos_ti_tidl_models_12_00_00_00_j784s4.tar.gz
```

### 6. Boot the target and run the demos

- **EVM Switch Settings:** Set the physical DIP switches on your TI EVM to SD Boot Mode. (Refer to the TI EVM Setup Guide for your specific SoC).

- **Insert & Power:** Insert the SD card into the EVM slot, connect a serial console (115200 baud), and power on the board.

- **Validate:** Once booted, run the out-of-box vision or TIDL demos located in /opt/vision_apps to verify the hardware accelerators are functional.
    ```bash
    cd /opt/vision_apps
    source ./vision_apps_init.sh

    # Object classification demo using TIDL
    ./run_app_tidl.sh
    ```



## Official Stable Release SDK and Documentation Links

- TI Processor SDK J721E: https://www.ti.com/tool/PROCESSOR-SDK-J721E 
- TI Processor SDK J721S2: https://www.ti.com/tool/PROCESSOR-SDK-J721S2
- TI Processor SDK J722S: https://www.ti.com/tool/PROCESSOR-SDK-J722S
- TI Processor SDK J784S4: https://www.ti.com/tool/PROCESSOR-SDK-J784S4
- TI Processor SDK J742S2: https://www.ti.com/tool/J742S2-PROCESSOR-SDK
- TI SDK training series: https://www.ti.com/video/series/processor-sdk-training-series.html 


## Acknowledgements

This project is based on Texas Instruments Jacinto Processor SDK ecosystem, including Processor SDK Linux, Processor SDK RTOS, and the associated training and documentation resources.


## License
This project is licensed under the terms of the TI End User License Agreement. Please refer to [LICENSE](LICENSE) and [manifest.html](manifest.html) for license information.