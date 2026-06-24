#
# Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
#Other Variables

# Inherit common build flags from root repo in SDK
-include $(PSDK_BUILDER_PATH)/build_flags.mak

# Supported values: PC, TI_DEVICE (SIMULATORS or EMULATOR )
TARGET_PLATFORM ?= TI_DEVICE

# Supported values: debug and release)
TARGET_BUILD    ?= release
# Supported C64T, C64P, C64, C66, C674, C67, C67P, m4
TARGET_CPU      ?= C71
# Supported values: yes and no)
BUILD_WITH_CUDA ?= no
# currently required to be set to yes
BUILD_CONFORMANCE_TEST ?=yes
# Enable GCOV tool for code coverage
GCOV_ENABLED ?= 0

# Enable compatibility to build on 9.2 SDK
ENABLE_SDK_9_2_COMPATIBILITY     ?= 0
# Enable compatibility to build on 10.0 SDK
ENABLE_SDK_10_0_COMPATIBILITY    ?= 0
# Enable compatibility to build on 10.1 SDK
ENABLE_SDK_10_1_COMPATIBILITY    ?= 0
# Enable compatibility to build on 11.0 SDK
ENABLE_SDK_11_0_COMPATIBILITY    ?= 0
# Enable compatibility to build on 11.1 SDK
ENABLE_SDK_11_1_COMPATIBILITY    ?= 0

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
  CGT_C7X_VERSION := 6.1.0.STS
  MMALIB_VERSION  := 11_02_01_04
else
  CGT_C7X_VERSION := 5.0.0.LTS
  MMALIB_VERSION  := 11_02_00_11
endif

PSDK_TOOLS_PATH     ?= $(HOME)/ti

CONCERTO_ROOT       ?=$(PSDK_BUILDER_PATH)/concerto
IVISION_PATH        ?=$(PSDK_INSTALL_PATH)/ivision

ifeq ($(TARGET_PLATFORM), PC)
MMALIB_PATH         ?=$(PSDK_INSTALL_PATH)/mmalib_$(MMALIB_VERSION)
PDK_INSTALL_PATH    ?=$(PSDK_INSTALL_PATH)/pdk/packages
# Below flag can be enabled in case opencv dependencies are present
BUILD_WITH_OPENCV    ?= 0
endif
MCU_PLUS_SDK_PATH   ?=$(PSDK_INSTALL_PATH)/mcu_plus_sdk

# Define MCU_SDK_PATH for TDA54 (separate from MCU_PLUS_SDK_PATH)
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
MCU_SDK_PATH := $(PSDK_INSTALL_PATH)/mcu_sdk
endif

OS_VERSION := $(shell cat /etc/os-release | grep VERSION_ID= | sed -e "s|VERSION_ID=\"||" | sed -e "s|\"||")

# DSP compiler required for tiovx kernels target dsp build
DSP_TOOLS           ?=$(PSDK_TOOLS_PATH)/ti-cgt-c7000_$(CGT_C7X_VERSION)

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

ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
  GCC_LINUX_ARM_ROOT  ?=$(PSDK_TOOLS_PATH)/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu
else ifeq ($(ENABLE_SDK_10_0_COMPATIBILITY), 1)
  GCC_LINUX_ARM_ROOT  ?=$(PSDK_TOOLS_PATH)/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-linux-gnu
else
  GCC_LINUX_ARM_ROOT ?= $(PSDK_INSTALL_PATH)/toolchain/sysroots/x86_64-arago-linux/usr
  CROSS_COMPILE_LINARO ?= aarch64-oe-linux/aarch64-oe-linux-
endif

LINUX_SYSROOT_ARM    =$(PSDK_INSTALL_PATH)/targetfs

VXLIB_PATH          ?=$(PSDK_INSTALL_PATH)/vxlib
PDK_INSTALL_PATH    ?=$(PSDK_INSTALL_PATH)/pdk/packages

TIDL_PATH           ?=$(abspath ../../)
TIOVX_PATH          ?=$(PSDK_INSTALL_PATH)/tiovx
VISION_APPS_PATH    ?=$(PSDK_INSTALL_PATH)/vision_apps
APP_UTILS_PATH      ?=$(PSDK_INSTALL_PATH)/app_utils
TIDL_PROTOBUF_PATH  ?=$(PSDK_INSTALL_PATH)/protobuf-3.21.12
CUDA_PATH           ?= /usr/local/cuda

LINUX_FS_PATH       ?=$(PSDK_INSTALL_PATH)/targetfs
# The TF-lite RT and ONNX RT paths are taken from targetfs
TF_REPO_PATH        ?=$(LINUX_FS_PATH)/usr/include/tensorflow

ONNX_REPO_PATH      ?=$(LINUX_FS_PATH)/usr/include/onnxruntime

TIDL_OPENCV_PATH    ?="$(PSDK_INSTALL_PATH)/opencv-4.1.0"

export QNX_SDP_VERSION ?= 710
ifeq ($(QNX_SDP_VERSION),700)
  export QNX_BASE ?= $(HOME)/qnx700
  export QNX_CROSS_COMPILER_TOOL ?= aarch64-unknown-nto-qnx7.0.0-
else
  export QNX_BASE ?= $(HOME)/qnx710
  export QNX_CROSS_COMPILER_TOOL ?= aarch64-unknown-nto-qnx7.1.0-
  # Adding this path for QNX SDP 7.1 which has a need to set the path
  # variable for the g++ tool to properly invloke the ld tool
  PATH := $(QNX_BASE)/host/linux/x86_64/usr/bin:$(PATH)
endif
export QNX_HOST ?= $(QNX_BASE)/host/linux/x86_64
export QNX_TARGET ?= $(QNX_BASE)/target/qnx7
export GCC_QNX_ROOT ?= $(QNX_HOST)/usr/bin
export GCC_QNX_ARM_ROOT ?= $(QNX_HOST)/usr/bin
export GCC_QNX_ARM ?= $(QNX_HOST)/usr/bin

#C7x Version (C7120/C7100)
#Compiles code for a particular C7x mode
C7x_HOSTEMU_COMPILER_STRING:=
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721E j721e))
C7X_TARGET ?= C71
C7X_VERSION ?= C7100
MPU_CPU=A72
RTOS_SDK ?= pdk
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721S2 j721s2 J784S4 j784s4 J742S2 j742s2))
C7X_TARGET ?= C7120
C7X_VERSION ?= C7120
MPU_CPU=A72
RTOS_SDK ?= pdk
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
C7X_TARGET ?= C7504
C7X_VERSION ?= C7504
MPU_CPU=A53
RTOS_SDK ?= mcu_plus_sdk
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
C7X_TARGET ?= C7524
C7X_VERSION ?= C7524
MPU_CPU=A53
RTOS_SDK ?= mcu_plus_sdk
C7x_HOSTEMU_COMPILER_STRING:=-MMA2_256
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
C7X_TARGET ?= C7604
C7X_VERSION ?= C7604
MPU_CPU=A720
RTOS_SDK ?= mcu_sdk
C7x_HOSTEMU_COMPILER_STRING:=-MMA3_1024
endif


ifeq ($(RTOS_SDK),mcu_plus_sdk)
  DMA_UTILS_PATH      ?=$(MCU_PLUS_SDK_PATH)/source/drivers/dmautils
  PDK_INSTALL_PATH    = .
  MCU_SDK_PATH        = .
else ifeq ($(RTOS_SDK),mcu_sdk)
  DMA_UTILS_PATH      ?=$(PSDK_INSTALL_PATH)/dmautils
  PDK_INSTALL_PATH    = .
  MCU_PLUS_SDK_PATH   = .
else
  # PDK
  DMA_UTILS_PATH      ?=$(PDK_INSTALL_PATH)/ti/drv/udma/dmautils
  MCU_PLUS_SDK_PATH   = .
  MCU_SDK_PATH        = .
endif
