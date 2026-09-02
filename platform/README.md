# platform

Platform initialization layer for TI Processor SDK applications. This repository
was extracted from `vision_apps` as part of SDK restructuring to enable independent
versioning, cleaner build boundaries, and reuse across SoCs.

## Supported SoCs

| SoC | RTOS Cores | HLOS |
|-----|------------|------|
| J721E | mcu1_0, mcu2_0, mcu2_1, mcu3_0, mcu3_1, c6x_1, c6x_2, c7x_1 | Linux, QNX |
| J721S2 | mcu1_0, mcu2_0, mcu2_1, mcu3_0, mcu3_1, c7x_1, c7x_2 | Linux, QNX |
| J722S | mcu1_0, mcu2_0, c7x_1, c7x_2 | Linux, QNX |
| J742S2 | mcu1_0, mcu2_0, mcu2_1, mcu3_0, mcu3_1, mcu4_0, mcu4_1, c7x_1, c7x_2, c7x_3 | Linux, QNX |
| J784S4 | mcu1_0, mcu2_0, mcu2_1, mcu3_0, mcu3_1, mcu4_0, mcu4_1, c7x_1, c7x_2, c7x_3, c7x_4 | Linux, QNX |
| TDA54 | mcu0–mcu4, rmcu0_0, rmcu0_1, rmcu1_0, rmcu1_1, rmcu2_0, rmcu2_1, c7x_1–c7x_4 | Linux, QNX |

## Repository Structure

```
platform/
├── Makefile                    # Top-level build entry point
├── build_flags.mak             # Delegates to sdk_builder/platform_build_flags.mak
├── target.mak                  # SYSIDIRS and target definitions
├── tools_path.mak              # Delegates to sdk_builder/tools_path.mak
├── platform.h                  # Top-level SoC include header
│
├── hlos/                       # High-level OS platform init (Linux / QNX)
│   ├── include/app_init.h      # Public app_init API for HLOS
│   └── src/
│       ├── app_init_linux.c    # Linux-side init: IPC, memory, remote cores
│       ├── app_init_qnx.c      # QNX-side init: equivalent sequence for QNX
│       ├── linux_lds/          # Linux firmware linker scripts
│       │   ├── atf_optee.lds
│       │   ├── tidtb_linux.lds
│       │   └── tikernelimage_linux.lds
│       └── qnx_lds/            # QNX firmware linker scripts
│           ├── atf_only.lds
│           ├── atf_optee.lds
│           └── ifs_qnx.lds
│
├── pc/                         # PC / VDK emulation platform init
│   ├── include/app_init.h      # Public app_init API for PC builds
│   └── src/
│       ├── app_init_pc.c       # PC (x86_64) platform init
│       ├── app_init_vdk.c      # Full VDK virtual platform init
│       ├── app_init_priv.h     # Private shared header
│       ├── vdk/                # Full VDK build rules
│       └── vdk_stub/           # VDK stub build rules
│
├── rtos/                       # RTOS platform init, one directory per SoC
│   └── <soc>/                  # e.g. j722s/, tda54/, am62a/, ...
│       ├── common/
│       │   ├── include/        # app_init.h, app_run.h, private headers
│       │   └── src/            # app_init.c, app_run.c (FreeRTOS/SafeRTOS/ThreadX)
│       ├── cores/
│       │   └── <core>/         # e.g. mcu1_0/, c7x_1/, ...
│       │       ├── include/core_cfg.h
│       │       └── src/
│       │           ├── main.c
│       │           ├── linker.cmd          # Active linker script
│       │           ├── linker_mem_map.cmd  # Memory map section for linker
│       │           ├── <soc>_linker_freertos.cmd
│       │           ├── <soc>_linker_freertos_mcuplus.cmd
│       │           ├── <soc>_mpu_cfg.c     # MPU region configuration
│       │           └── concerto.mak
│       └── concerto_<core>_inc.mak         # Per-SoC concerto include per core
│
├── memory_map/                 # Auto-generated memory maps, one directory per SoC
│   └── <soc>/                  # e.g. j722s/, tda54/, am62a/, ...
│       ├── app_mem_map.h                   # C header with memory region definitions
│       ├── system_memory_map.html          # Human-readable memory map table
│       ├── gen_linker_mem_map.py           # Script to regenerate artifacts
│       ├── gen_linker_mem_map_safertos.py  # SafeRTOS variant (where applicable)
│       └── k3-<soc>-rtos-memory-map.dtsi  # Linux DT overlay for RTOS regions
│
├── utils/                      # Shared utility modules
│   ├── dss/                    # Display Subsystem (DSS/DCTRL) utility
│   ├── ethfw/                  # Ethernet firmware RTOS-side management
│   ├── ipc/                    # IPC resource table and trace utility
│   ├── mem/                    # Heap and shared memory management
│   ├── mpu/                    # MPU configuration (FreeRTOS/SafeRTOS, MCU Plus/PDK)
│   ├── sciserver/              # SCI server (DMSC/TIFS message handling)
│   └── vdk-utils/              # VDK virtual platform configs and boot scripts
│       ├── scripts/qnx/        # QNX filesystem and boot scripts for TDA54 VDK
│       └── vpconfigs/          # VPX virtual platform configurations
│           ├── psdk-linux/     # PSDK Linux VDK config
│           ├── psdk-rtos/      # PSDK RTOS VDK config
│           ├── psdk-xen/       # PSDK Xen VDK config
│           ├── psdk-xen-static/# PSDK Xen static VDK config
│           └── tda54-vdk-qnx/  # TDA54 QNX VDK config
│
└── tools/
    └── PyTI_PSDK_RTOS/         # Python package for memory map generation
        └── ti_psdk_rtos_tools/ # Generators: C header, DTS, HTML table, linker cmd
```

## Build System

The platform repo uses the TI Concerto build system and delegates flag and path
configuration to `sdk_builder` (expected at `../sdk_builder` relative to this repo).

### Prerequisites

- TI Processor SDK installed with `sdk_builder` at `$(PSDK_PATH)/sdk_builder`
- Toolchains (resolved via `sdk_builder/tools_path.mak`):
  - `TIARMCGT_LLVM` — ARM LLVM toolchain (R5F, R52+, M55 cores)
  - `CGT7X` — TI C7000 CGT (C7x DSP cores)
  - `CGT6X` — TI C6000 CGT (C66x DSP cores, J721E only)
  - `GCC_LINUX_ARM` — GCC for Linux/ARM MPU
  - `GCC_QNX_ARM` — GCC for QNX/ARM MPU
  - `GCC_LINUX` — GCC for PC (x86_64) emulation builds

### Building

```bash
# Build all RTOS + HLOS targets for a given SoC (debug and release)
make PROFILE=all

# Build debug only
make PROFILE=debug

# Build release only
make PROFILE=release

# Build PC emulation targets
make BUILD_EMULATION_MODE=yes PROFILE=debug

# Clean build outputs
make clean
```

Build flags (`TARGET_SOC`, `RTOS`, `PROFILE`, `BUILD_LINUX_MPU`, etc.) are defined
in `sdk_builder/platform_build_flags.mak` and can be overridden on the command line.

### IPC Core Change Detection

The top-level Makefile tracks `ENABLED_IPC_CORES` in `.build_core.bak`. If the
set of IPC-enabled cores changes between builds, `app_init.c` (RTOS and HLOS) is
touch-forced to trigger a rebuild that picks up the updated core list.

## Memory Maps

Memory map artifacts under `memory_map/<soc>/` are auto-generated by
`tools/PyTI_PSDK_RTOS`. Do not edit `app_mem_map.h` or `.dtsi` files manually.

To regenerate for a SoC:

```bash
cd memory_map/<soc>
python3 gen_linker_mem_map.py
```

### Installing the PyTI_PSDK_RTOS tool

```bash
pip install -e tools/PyTI_PSDK_RTOS --user
```

## Utility Modules (`utils/`)

| Module | Purpose |
|--------|---------|
| `dss` | Display Subsystem initialization and DCTRL configuration |
| `ethfw` | RTOS-side Ethernet firmware init and lifecycle management |
| `ipc` | IPC resource table (`ipc_rsctable.c`), trace buffer, and core IPC init |
| `mem` | Heap allocation and common mem interface across all SoCs |
| `mpu` | MPU region setup; FreeRTOS and SafeRTOS variants for mcu_plus_sdk and PDK |
| `sciserver` | System Controller (DMSC/TIFS) message server initialization |
| `vdk-utils` | VPX virtual platform configs (`.vpcfg`) and board-support Python scripts for VDK simulation runs |

## VDK Virtual Platform Configs

The `utils/vdk-utils/vpconfigs/` directory contains configurations for the
TI VPX virtual platform (VDK). Each config includes:

- `*.vpcfg` — Virtual platform configuration file
- `scripts/board-support/` — Core init scripts (`init_c7x.py`, `init_r52.py`, etc.)
- `scripts/optional/` — Optional features (OSkit, display, simulation speed)
- `vpx_startup_*.py` — VPX startup scripts for display and GPU variants

| Config | Description |
|--------|-------------|
| `psdk-linux` | PSDK Linux boot on VDK |
| `psdk-rtos` | PSDK RTOS boot on VDK |
| `psdk-xen` | PSDK Xen hypervisor on VDK |
| `psdk-xen-static` | PSDK Xen static configuration on VDK |
| `tda54-vdk-qnx` | TDA54 QNX on VDK |

## License

Copyright (c) 2018–2026 Texas Instruments Incorporated.
Licensed under the TI Limited License. See individual source file headers for
complete license terms.
