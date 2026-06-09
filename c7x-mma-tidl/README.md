# C7X-MMA-TIDL Repository

## Setup Instructions

Follow these steps to set up the TIDL repository for development:

### 1. Clone the Repository

```bash
git clone <repository-url>
cd c7x-mma-tidl
```

### 2. Initialize and Update Git ARM-TIDL submodule

The repository uses arm-tidl as git submodules. Initialize and update them with:

```bash
git submodule init
git submodule update
```

### 3. Configure the Repository for Your Target Device

Before building, configure the repository for your specific SOC using the `configureRepoForDevice.py` script:

```bash
cd ti_dl/utils/python
python configureRepoForDevice.py <SOC>
cd ../../../
```

Where `<SOC>` is one of:
- j721e
- j721s2
- j784s4
- am62a
- j722s
- j742s2
- tda54

### 4. Build the Project

Use the provided `build.sh` script to build the project:

```bash
./build.sh [options]
```

#### Build Options:

- `--scrub`: Scrub and clean build artifacts.
- `--psdk_install_path`: Path to the TI Processor SDK installation. Default: <repository-path>/../
- `--target_build`: Specify build profile (release, debug, all). Default: release
  - When set to `all`, the build process will compile both debug and release configurations
- `--target_platform`: Specify target platform (TI_DEVICE, PC, all). Default: TI_DEVICE
  - When set to `PC`, the build process will compile for PC and update the symbolic links under the `tidl_tools` path
  - When set to `all`, the build process will compile for both TI_DEVICE and PC platforms
- `-j<N>`: Use parallel build with N jobs (e.g., -j8, -j16). Significantly speeds up build time on multi-core systems.
  - If not specified, the build script automatically uses (number of processors - 2) for parallel builds when more than 2 processors are available to avoid excessive system load
- `--install`: Copy built libraries, headers and tidl_tools tarball. Do not use during development
- `--install_path`: Path to copy built libraries, headers and tidl_tools tarball. Do not use during development

#### SDK Compatibility Requirements:

> **IMPORTANT**: The build mode of your SDK must match the build mode of TIDL:

- When building for `TI_DEVICE`, ensure SDK is built in `TARGET_MODE`
- When building for `PC`, ensure SDK is built in `HOST_EMULATION` mode
- For `release` build, ensure SDK is built in `release` mode
- For `debug` build, ensure SDK is built in `debug` mode

By default SDK is only build in `TARGET_MODE` with `release` profile. It is recommended to build the SDK in all modes during the initial one-time setup by modifying the `sdk_builder/build_flags.mak` file. 

#### Examples:

Build for release on TI device:
```bash
./build.sh -j8
```

Build for PC emulation with release configuration:
```bash
./build.sh --target_platform PC -j8
```

Build all configurations (release + debug) for both TI_DEVICE and PC:
```bash
./build.sh --target_platform all --target_build all
```