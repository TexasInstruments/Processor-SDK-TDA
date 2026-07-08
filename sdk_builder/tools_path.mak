PSDK_PATH ?= $(abspath ..)

# Compilers
PSDK_TOOLS_PATH ?= $(HOME)/ti
ifneq ($(wildcard $(PSDK_PATH)/.component_versions_$(SOC)_env),)
    include $(PSDK_PATH)/.component_versions_$(SOC)_env
endif

TIARMCGT_LLVM_ROOT ?= $(PSDK_TOOLS_PATH)/ti-cgt-armllvm_$(CGT_ARMLLVM_VERSION)
CGT7X_ROOT ?= $(PSDK_TOOLS_PATH)/ti-cgt-c7000_$(CGT_C7X_VERSION)
CGT6X_ROOT ?= $(PSDK_TOOLS_PATH)/ti-cgt-c6000_$(CGT_C6X_VERSION)
GCC_LINUX_ARM_ROOT ?= $(PSDK_PATH)/toolchain/sysroots/x86_64-arago-linux/usr
CROSS_COMPILE_LINARO ?= aarch64-oe-linux/aarch64-oe-linux-

# SDK RTOS Components
LINUX_SYSROOT_ARM ?= $(PSDK_PATH)/targetfs
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder
CONCERTO_ROOT ?= $(PSDK_BUILDER_PATH)/concerto
PDK_PATH ?= $(PSDK_PATH)/pdk
MCU_PLUS_SDK_PATH ?= $(PSDK_PATH)/mcu_plus_sdk
MCU_SDK_PATH ?= $(PSDK_PATH)/mcu_sdk
DMA_UTILS_PATH ?= $(PSDK_PATH)/dmautils
PDK_QNX_PATH ?= $(PSDK_PATH)/psdkqa/pdk
VXLIB_PATH ?= $(PSDK_PATH)/vxlib
MATHLIB_PATH ?= $(PSDK_PATH)/mathlib_c66x_3_1_2_1

ifeq ($(SOC),am62a)
    MMALIB_PATH ?= $(PSDK_PATH)/mmalib_11_01_00_06
else ifeq ($(SOC),tda54)
    MMALIB_PATH ?= $(PSDK_PATH)/mmalib_11_02_01_04
else
    MMALIB_PATH ?= $(PSDK_PATH)/mmalib_11_02_00_11
endif

IVISION_PATH ?= $(PSDK_PATH)/ivision
TIDL_PATH ?= $(PSDK_PATH)/c7x-mma-tidl
APP_UTILS_PATH ?= $(PSDK_PATH)/app_utils
TIOVX_PATH ?= $(PSDK_PATH)/tiovx

ifeq ($(SOC), j721e)
    VPAC_C_MODELS_PATH ?= $(PSDK_PATH)/vhwa_c_models/vpac1
else ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4 j742s2 tda54))
    VPAC_C_MODELS_PATH ?= $(PSDK_PATH)/vhwa_c_models/vpac3
else ifeq ($(SOC), $(filter $(SOC), am62a j722s))
    VPAC_C_MODELS_PATH ?= $(PSDK_PATH)/vhwa_c_models/vpac3l
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j784s4 j742s2 j722s tda54))
    DMPAC_C_MODELS_PATH ?= $(PSDK_PATH)/vhwa_c_models/dmpac
endif

IMAGING_PATH ?= $(PSDK_PATH)/imaging
VIDEO_IO_PATH ?= $(PSDK_PATH)/video_io
VISION_APPS_PATH ?= $(PSDK_PATH)/vision_apps
MCUSW_PATH ?= $(PSDK_PATH)/mcusw
PTK_PATH ?= $(PSDK_PATH)/ti-perception-toolkit
TIADALG_PATH ?= $(PSDK_PATH)/tiadalg
GLM_PATH ?= $(PSDK_PATH)/glm
ifeq ($(SOC), $(filter $(SOC), am62a j722s))
ETHFW_PATH ?= $(PSDK_PATH)/mcu_plus_sdk/source/networking/ethfw
else
ETHFW_PATH ?= $(PSDK_PATH)/ethfw
endif
TI_SECURE_DEV_PKG ?= $(PSDK_PATH)/core-secdev-k3
EDGEAI_UTILS_PATH ?= $(PSDK_PATH)/edgeai/edgeai-apps-utils/
EDGEAI_KERNELS_PATH ?= $(PSDK_PATH)/edgeai/edgeai-tiovx-kernels/

ifeq ($(SOC),j722s)
    ifeq ($(RTOS),SAFERTOS)
        include $(MCU_PLUS_SDK_PATH)/imports.mak
        export SAFERTOS_R5F_VARIANT ?= EVAL_-_TI_Clang_TDA4VEN
        export SAFERTOS_C7X_VARIANT ?= EVAL_-_TI_CGT_TDA4VEN
        export SAFERTOS_KERNEL_INSTALL_PATH_r5f = $(PSDK_PATH)/safertos_$(SOC)_r5f_$(SAFERTOS_VERSION_r5f)/safertos_$(SAFERTOS_VERSION_r5f)_$(SAFERTOS_R5F_VARIANT)
        export SAFERTOS_KERNEL_INSTALL_PATH_c7x = $(PSDK_PATH)/safertos_$(SOC)_c7x_$(SAFERTOS_VERSION_c7x)/safertos_$(SAFERTOS_VERSION_c7x)_$(SAFERTOS_C7X_VARIANT)
    endif
endif

# This is required to be set when pulling in the safertos_version
ifeq ($(VDK), yes)
BOARD=$(SOC)_vdk
else
    BOARD=$(SOC)_evm
endif

ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j784s4 j742s2))
    ifneq ($(wildcard $(PDK_PATH)),)
        include $(PDK_PATH)/packages/ti/build/safertos_version.mk

        ifeq ($(SAFERTOS_$(SOC)_r5f_INSTALL_PATH),)
            export SAFERTOS_KERNEL_INSTALL_PATH_r5f = $(PSDK_PATH)/safertos_$(SOC)_r5f_$(SAFERTOS_VERSION_r5f)
        else
            export SAFERTOS_KERNEL_INSTALL_PATH_r5f = $(SAFERTOS_$(SOC)_r5f_INSTALL_PATH)
        endif
        ifeq ($(SAFERTOS_$(SOC)_c7x_INSTALL_PATH),)
            export SAFERTOS_KERNEL_INSTALL_PATH_c7x = $(PSDK_PATH)/safertos_$(SOC)_c7x_$(SAFERTOS_VERSION_c7x)
        else
            export SAFERTOS_KERNEL_INSTALL_PATH_c7x = $(SAFERTOS_$(SOC)_c7x_INSTALL_PATH)
        endif
        ifeq ($(SOC),j721e)
            ifeq ($(SAFERTOS_$(SOC)_c66_INSTALL_PATH),)
            export SAFERTOS_KERNEL_INSTALL_PATH_c66 = $(PSDK_PATH)/safertos_$(SOC)_c66_$(SAFERTOS_VERSION_c66)
            else
            export SAFERTOS_KERNEL_INSTALL_PATH_c66 = $(SAFERTOS_$(SOC)_c66_INSTALL_PATH)
            endif
        endif
    endif
endif

BUILD_OS ?= Linux

ifeq ($(BUILD_OS),Linux)
    GCC_LINUX_ROOT ?= /usr/
endif

LINUX_FS_PATH ?= $(PSDK_PATH)/targetfs/
LINUX_FS_BOOT_PATH ?= $(PSDK_PATH)/bootfs/
LINUX_SD_FS_ROOT_PATH ?= /media/$(USER)/root
LINUX_SD_FS_BOOT_PATH ?= /media/$(USER)/boot

ifeq ($(SOC),j721e)
  export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-j721e-evm-11_02_01_03
else ifeq ($(SOC),j721s2)
  export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-j721s2-evm-11_02_01_03
else ifeq ($(SOC),j784s4)
  export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-j784s4-evm-11_02_01_03
else ifeq ($(SOC),j742s2)
  export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-j742s2-evm-11_02_01_03
else ifeq ($(SOC),j722s)
  export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-j722s-evm-11_02_01_03
else ifeq ($(SOC),tda54)
    export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-tda54-vdk-12_00_01_04
else ifeq ($(SOC),am62a)
    ifeq ($(TISDK_IMAGE), edgeai)
        export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-edgeai-am62a-evm-11.02.09.03
    else
        export PSDK_LINUX_PATH ?= $(HOME)/ti-processor-sdk-linux-adas-am62a-evm-11.02.11.02
    endif
endif

# QNX Paths
ifeq ($(SOC), $(filter $(SOC), tda54))
    export PSDK_QNX_PATH ?= $(PSDK_PATH)/psdk_qnx
    export PSDK_QNX_BUILD_PATH ?= $(PSDK_QNX_PATH)
    export QNX_FS_PATH ?= $(PSDK_QNX_PATH)/output/qnxfs
    export QNX_BOOT_PATH ?= $(PSDK_QNX_PATH)/output/bootfs/
    export QNX_STAGE_PATH ?= $(PSDK_QNX_PATH)/output/stage
else
    export PSDK_QNX_PATH ?= $(PSDK_PATH)/psdkqa
    export PSDK_QNX_BUILD_PATH ?= $(PSDK_QNX_PATH)/qnx
    export QNX_FS_PATH ?= $(PSDK_QNX_PATH)/qnxfs
    export QNX_BOOT_PATH ?= $(PSDK_QNX_PATH)/bootfs/
    export QNX_STAGE_PATH ?= $(PSDK_QNX_PATH)/stage
    export PDK_QNX_PATH ?= $(PSDK_QNX_PATH)/pdk
endif

# The default SDP version is QNX SDP 8.0.0 for all platforms (except AM62A)
# Override this in your environment to 7.1.0 as needed.
ifeq ($(SOC),am62a)
    export QNX_SDP_VERSION ?= 710
else
    export QNX_SDP_VERSION ?= 800
endif

ifeq ($(QNX_SDP_VERSION),800)
    export QNX_BASE ?= $(HOME)/qnx800
    export QNX_CROSS_COMPILER_TOOL ?= aarch64-unknown-nto-qnx8.0.0-
    export QNX_TARGET ?= $(QNX_BASE)/target/qnx
    PATH := $(QNX_BASE)/host/linux/x86_64/usr/bin:$(PATH)
else
    export QNX_BASE ?= $(HOME)/qnx710
    export QNX_CROSS_COMPILER_TOOL ?= aarch64-unknown-nto-qnx7.1.0-
    # Adding this path for QNX SDP 7.1 which has a need to set the path
    # variable for the g++ tool to properly invloke the ld tool
    PATH := $(QNX_BASE)/host/linux/x86_64/usr/bin:$(PATH)
    export QNX_TARGET ?= $(QNX_BASE)/target/qnx7
endif

export QNX_HOST ?= $(QNX_BASE)/host/linux/x86_64
export GCC_QNX_ROOT ?= $(QNX_HOST)/usr/bin
export GCC_QNX_ARM_ROOT ?= $(QNX_HOST)/usr/bin
export GCC_QNX_ARM ?= $(QNX_HOST)/usr/bin

QNX_SD_FS_ROOT_PATH ?= /media/$(USER)/rootfs
QNX_SD_FS_QNX_PATH ?= /media/$(USER)/qnxfs
QNX_SD_FS_BOOT_PATH ?= /media/$(USER)/boot
