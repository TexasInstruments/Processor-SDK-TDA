#
# Copyright (c) {2015 - 2026} Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
# *       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
# *       any redistribution and use are licensed by TI for use only with TI Devices.
#
# *       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
# *       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
# *       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

# SDK backward-compatibility build flags (default off; can be overridden from environment)
ENABLE_SDK_9_2_COMPATIBILITY     ?= 0
ENABLE_SDK_10_0_COMPATIBILITY    ?= 0
ENABLE_SDK_10_1_COMPATIBILITY    ?= 0
ENABLE_SDK_11_0_COMPATIBILITY    ?= 0
ENABLE_SDK_11_1_COMPATIBILITY    ?= 0

# C7x compiler toolchain and MMALIB version (TDA54 uses a different CGT STS release)
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
  CGT_C7X_VERSION := 6.1.0.STS
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
  CGT_C7X_VERSION := 6.2.0.LTS
else
  CGT_C7X_VERSION := 5.0.0.LTS
endif

PSDK_TOOLS_PATH     ?= $(HOME)/ti
CGT7X_ROOT          := "$(PSDK_TOOLS_PATH)/ti-cgt-c7000_$(CGT_C7X_VERSION)"

# Check for mmalib in local repo first (standalone build), then fall back to SDK install path
MMALIB_LOCAL_PATH  := $(ARM_TIDL_PATH)/../mmalib-kernels/mmalib
MMALIB_SDK_PATH    := $(PSDK_INSTALL_PATH)/c7x-mma-tidl/mmalib-kernels/mmalib
MMALIB_PATH        := $(if $(wildcard $(MMALIB_LOCAL_PATH)),$(MMALIB_LOCAL_PATH),$(MMALIB_SDK_PATH))
$(info MMALIB_PATH is: $(MMALIB_PATH))

# RTOS SDK type derived from TARGET_SOC
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a J722S j722s))
RTOS_SDK ?= mcu_plus_sdk
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
RTOS_SDK ?= mcu_sdk
else
RTOS_SDK ?= pdk
endif

#Enable performance modelling flow
ENABLE_PERFORMANCE_MODELLING     ?= 0

# Enforce mutual exclusion of SDK compatibility flags.
ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
  ENABLE_SDK_10_0_COMPATIBILITY = 0
  ENABLE_SDK_10_1_COMPATIBILITY = 0
  ENABLE_SDK_11_0_COMPATIBILITY = 0
  ENABLE_SDK_11_1_COMPATIBILITY = 0
else ifeq ($(ENABLE_SDK_10_0_COMPATIBILITY), 1)
  ENABLE_SDK_9_2_COMPATIBILITY = 0
  ENABLE_SDK_10_1_COMPATIBILITY = 0
  ENABLE_SDK_11_0_COMPATIBILITY = 0
  ENABLE_SDK_11_1_COMPATIBILITY = 0
else ifeq ($(ENABLE_SDK_10_1_COMPATIBILITY), 1)
  ENABLE_SDK_9_2_COMPATIBILITY = 0
  ENABLE_SDK_10_0_COMPATIBILITY = 0
  ENABLE_SDK_11_0_COMPATIBILITY = 0
  ENABLE_SDK_11_1_COMPATIBILITY = 0
else ifeq ($(ENABLE_SDK_11_0_COMPATIBILITY), 1)
  ENABLE_SDK_9_2_COMPATIBILITY = 0
  ENABLE_SDK_10_0_COMPATIBILITY = 0
  ENABLE_SDK_10_1_COMPATIBILITY = 0
  ENABLE_SDK_11_1_COMPATIBILITY = 0
else ifeq ($(ENABLE_SDK_11_1_COMPATIBILITY), 1)
  ENABLE_SDK_9_2_COMPATIBILITY = 0
  ENABLE_SDK_10_0_COMPATIBILITY = 0
  ENABLE_SDK_10_1_COMPATIBILITY = 0
  ENABLE_SDK_11_0_COMPATIBILITY = 0
endif


# Enable the new platform/ directory structure (ADASVISION-7321) in place of the
# legacy vision_apps/ layout. Set to 'no' when building against an SDK that still
# ships the old vision_apps/tivision_apps layout.
#
# AM62A SDKs currently does not have any plan to absorbe the afromentioned changes
# hence setting to no by default.
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
ENABLE_NEW_VISION_APPS_STRUCTURE ?= no
else
ENABLE_NEW_VISION_APPS_STRUCTURE ?= yes
endif

ifeq ($(ENABLE_NEW_VISION_APPS_STRUCTURE),yes)
PLATFORM_PATH       ?=$(PSDK_INSTALL_PATH)/platform
else
VISION_APPS_PATH    ?=$(PSDK_INSTALL_PATH)/vision_apps
endif

# Compat-aware toolchain selection.  Set before the sdk_builder include so its
# GCC_LINUX_ARM_ROOT ?= default is a no-op when an override is active.
ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
  GCC_LINUX_ARM_ROOT ?= $(PSDK_TOOLS_PATH)/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu
else ifeq ($(ENABLE_SDK_10_0_COMPATIBILITY), 1)
  GCC_LINUX_ARM_ROOT ?= $(PSDK_TOOLS_PATH)/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-linux-gnu
endif

# QNX SDP 7.0.0 (legacy) — set before sdk_builder so its else-branch (qnx710)
# default is a no-op when QNX_SDP_VERSION=700 is active.
ifeq ($(QNX_SDP_VERSION),700)
  export QNX_BASE               ?= $(HOME)/qnx700
  export QNX_CROSS_COMPILER_TOOL ?= aarch64-unknown-nto-qnx7.0.0-
endif

# Pull in all SDK-wide paths.  This is the single authoritative source for
# MMALIB_PATH (guarded by ENABLE_NEW_TIDL_STRUCTURE), TIDL_PATH, IVISION_PATH,
# MCU_PLUS_SDK_PATH, TIOVX_PATH, LINUX_FS_PATH, GCC_LINUX_ARM_ROOT, etc.
include $(PSDK_BUILDER_PATH)/tools_path.mak

# PDK install path (packages subdirectory), used by DMA_UTILS_PATH PDK branch below.
PDK_INSTALL_PATH    ?= $(PDK_PATH)/packages

# Route DMA_UTILS_PATH to the correct location for the active RTOS SDK.
# Use := to override the flat sdk_builder default ($(PSDK_PATH)/dmautils).
ifeq ($(RTOS_SDK),mcu_plus_sdk)
  DMA_UTILS_PATH    := $(MCU_PLUS_SDK_PATH)/source/drivers/dmautils
  PDK_INSTALL_PATH  = .
  MCU_SDK_PATH      = .
else ifeq ($(RTOS_SDK),mcu_sdk)
  DMA_UTILS_PATH    := $(PSDK_INSTALL_PATH)/dmautils
  PDK_INSTALL_PATH  = .
  MCU_PLUS_SDK_PATH = .
else
  DMA_UTILS_PATH    := $(PDK_INSTALL_PATH)/ti/drv/udma/dmautils
  MCU_PLUS_SDK_PATH = .
  MCU_SDK_PATH      = .
endif

# TIDL-specific paths not provided by sdk_builder/tools_path.mak.
OS_VERSION          := $(shell cat /etc/os-release | grep VERSION_ID= | sed -e "s|VERSION_ID=\"||" | sed -e "s|\"||")
ifeq ($(OS_VERSION),18.04)
TIDL_GCC_VERSION    ?= 5
else ifeq ($(OS_VERSION),22.04)
TIDL_GCC_VERSION    ?= 11
else ifeq ($(OS_VERSION),24.04)
TIDL_GCC_VERSION    ?= 13
endif
BIOS_PATH           ?= "$(PSDK_INSTALL_PATH)/bios_6_83_02_07"
XDCTOOLS_PATH       ?= "$(PSDK_INSTALL_PATH)/xdctools_3_61_04_40_core"
TIDL_OPENCV_PATH    ?= "$(PSDK_INSTALL_PATH)/opencv-4.1.0"
TIDL_PROTOBUF_PATH  ?= $(PSDK_INSTALL_PATH)/protobuf-3.21.12
TIDL_FLATBUF_PATH   ?= "$(PSDK_INSTALL_PATH)/flatbuffers-1.12.0"
TIDL_GRAPHVIZ_PATH  ?= "/usr"
CUDA_PATH           ?= /usr/local/cuda
TF_REPO_PATH        ?= $(LINUX_FS_PATH)/usr/include/tensorflow
ONNX_REPO_PATH      ?= $(LINUX_FS_PATH)/usr/include/onnxruntime
TVM_HOME            ?= $(LINUX_FS_PATH)/usr/include/tvm
