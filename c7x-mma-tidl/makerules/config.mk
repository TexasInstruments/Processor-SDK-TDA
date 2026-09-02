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

# TARGET_PLATFORM      :  Used to select the target platform. PC(Host emulation) or the TI device.
# TARGET_CPU           :  Used select Particular TI compute core. This applicable in PC emulation
#                         mode as well to select the Host emulation packages
# TARGET_BUILD         :  used to select Debug or Release build

# TIDL_BUILD_FOR_LOKI  :  Temporary flag. Will be removed once VLAB simulator matures.
# GCOV_ENABLED         :  Used to enable GCOV tool for TIDL code coverage
#
# Example usages
# 1. Build for C7x Target : gmake or  gmake TARGET_PLATFORM=TI_DEVICE TARGET_CPU=C71
# 2. Build for C7x Host Emualtion : gmake TARGET_PLATFORM=PC TARGET_CPU=C71
# 3. Build for C66 Host Emualtion : gmake TARGET_PLATFORM=PC TARGET_CPU=C66
# 4. Build for C66 Target : gmake TARGET_PLATFORM=TI_DEVICE TARGET_CPU=C66
# 5. Build for C7x Target debug build : gmake TARGET_PLATFORM=TI_DEVICE TARGET_CPU=C71 TARGET_BUILD=debug

# Inherit common build flags from root repo in SDK
PSDK_BUILDER_PATH ?= $(PSDK_INSTALL_PATH)/sdk_builder

include $(PSDK_BUILDER_PATH)/build_flags.mak

# TIDL outputs files (trace files, output files)
# to have platform prefix
# Supported values: 0, 1
PREFIXED_OUTPUTS ?= 0

# Default TI_DEVELOPER_BUILD
# Supported values: 0, 1
DEVELOPER_BUILD ?= 0

# Default TI_DEVICE
# Supported values: PC, TI_DEVICE (SIMULATORS or EMULATOR )
#TARGET_PLATFORM ?= PC
TARGET_PLATFORM ?= TI_DEVICE


BUILD_WITH_OPENACC     ?= no
BUILD_WITH_CUDA        ?= no
TIDL_BUILD_FOR_LOKI    ?= 0
TIDL_ENABLE_OPERATOR_WITH_UNIMPLEMENTED_KERNELS    ?= 0

# Kernels supported by the inference engine (passed to tidl-kernels CMake build).
# =ON  : kernel is built and linked into libtidl_kernels
# =OFF : kernel is not yet supported; add =ON here when inference engine gains support
TIDL_SUPPORTED_KERNEL_FLAGS ?= \
    -DKERNEL_ARGOP=ON \
    -DKERNEL_BATCHNORM=ON \
    -DKERNEL_BATCHRESHAPE=ON \
    -DKERNEL_CAST=ON \
    -DKERNEL_COLORCONVERSION=ON \
    -DKERNEL_COMPARISONOP=ON \
    -DKERNEL_CONCAT=ON \
    -DKERNEL_DEFORMCONV=ON \
    -DKERNEL_ELTWISE=ON \
    -DKERNEL_FLATTEN=ON \
    -DKERNEL_GATHER=ON \
    -DKERNEL_GATHERND=ON \
    -DKERNEL_GATHERELEMENTS=ON \
    -DKERNEL_GLOBALAVGPOOL=ON \
    -DKERNEL_GLOBALMAXPOOL=ON \
    -DKERNEL_GRIDSAMPLE=ON \
    -DKERNEL_GRU=ON \
    -DKERNEL_LSTM=ON \
    -DKERNEL_RNN=ON \
    -DKERNEL_LAYERNORM=ON \
    -DKERNEL_LOGICALOP=ON \
    -DKERNEL_NONFINITE=ON \
    -DKERNEL_NONZERO=ON \
    -DKERNEL_ODFINDVALIDLOCANDSCORE=ON \
    -DKERNEL_REDUCE=ON \
    -DKERNEL_RESIZE=ON \
    -DKERNEL_SCATTERELEMENTS=ON \
    -DKERNEL_SIGN=ON \
    -DKERNEL_SOFTMAX=ON \
    -DKERNEL_SPATIALAVGPOOL=ON \
    -DKERNEL_SPATIALMAXPOOL=ON \
    -DKERNEL_TOPK=ON \
    -DKERNEL_TRANSPOSE=ON \
    -DKERNEL_WHERE=ON

TIDL_HOST_CCS          ?= 0
TIDL_BIOS_BUILD        ?= 1
BUILD_LIDAR_PREPROC    ?= 0
GCOV_ENABLED           ?= 0

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

#Enable performance modelling flow 
ENABLE_PERFORMANCE_MODELLING     ?= 0

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
  CGT_C7X_VERSION := 6.1.0.STS
  MMALIB_VERSION  := 11_02_01_04
else
  CGT_C7X_VERSION := 5.0.0.LTS
  MMALIB_VERSION  := 12_01_00_01
endif

ifdef SystemRoot
PSDK_TOOLS_PATH     ?= "C:/ti"
BIOS_PATH           ?="$(PSDK_INSTALL_PATH)\bios_6_83_02_07"
XDCTOOLS_PATH       ?="$(PSDK_INSTALL_PATH)\xdctools_3_61_04_40_core"
IVISION_PATH        ?="$(PSDK_INSTALL_PATH)\ivision"
UTILS_PATH          ?="$(PSDK_INSTALL_PATH)\ccs910\ccs\utils\cygwin"
TIDL_PROTOBUF_PATH  ?="$(PSDK_INSTALL_PATH)\protobuf-3.11.3_msvc_2015_x64"
TIDL_OPENCV_PATH    ?=$(PSDK_INSTALL_PATH)\opencv_3.1.0_msvc_2015_x64\opencv\sources
DSP_TOOLS           ?=$(PSDK_TOOLS_PATH)\ti-cgt-c7000_$(CGT_C7X_VERSION)
TIDL_FLATBUF_PATH   ?=$(PSDK_INSTALL_PATH)\flatbuffers-1.12.0
MMALIB_PATH         ?=$(PSDK_INSTALL_PATH)\mmalib_$(MMALIB_VERSION)
PDK_INSTALL_PATH    ?=$(PSDK_INSTALL_PATH)\pdk\packages
CONCERTO_ROOT       ?=$(PSDK_BUILDER_PATH)\concerto
MCU_PLUS_SDK_PATH   ?=$(PSDK_INSTALL_PATH)\mcu_plus_sdk
MCU_SDK_PATH        ?=$(PSDK_INSTALL_PATH)\mcu_sdk
TIOVX_PATH          ?=$(PSDK_INSTALL_PATH)\tiovx
PLATFORM_PATH       ?=$(PSDK_INSTALL_PATH)\platform

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

TIDL_GRAPHVIZ_PATH  ?="$(PSDK_INSTALL_PATH)\graphviz-2.38_x64"
# Location of the CUDA Toolkit
CUDA_PATH ?= "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v9.0/"
CUDNN_PATH ?= "D:/work/cuDNN/cudnn-9.0-v7.0/"
else
PSDK_TOOLS_PATH     ?= $(HOME)/ti
# Set the default gcc based on the version of Ubuntu
OS_VERSION := $(shell cat /etc/os-release | grep VERSION_ID= | sed -e "s|VERSION_ID=\"||" | sed -e "s|\"||")
ifeq ($(OS_VERSION),18.04)
	TIDL_GCC_VERSION?=5
else ifeq  ($(OS_VERSION),22.04)
	TIDL_GCC_VERSION?=11
else ifeq  ($(OS_VERSION),24.04)
	TIDL_GCC_VERSION?=13
endif

BIOS_PATH           ?="$(PSDK_INSTALL_PATH)/bios_6_83_02_07"
XDCTOOLS_PATH       ?="$(PSDK_INSTALL_PATH)/xdctools_3_61_04_40_core"
IVISION_PATH        ?="$(PSDK_INSTALL_PATH)/ivision"
DSP_TOOLS           ?="$(PSDK_TOOLS_PATH)/ti-cgt-c7000_$(CGT_C7X_VERSION)"
MMALIB_PATH         ?="$(PSDK_INSTALL_PATH)/mmalib_$(MMALIB_VERSION)"
PDK_INSTALL_PATH    ?="$(PSDK_INSTALL_PATH)/pdk/packages"
CONCERTO_ROOT       ?=$(PSDK_BUILDER_PATH)/concerto
MCU_PLUS_SDK_PATH   ?=$(PSDK_INSTALL_PATH)/mcu_plus_sdk
MCU_SDK_PATH        ?=$(PSDK_INSTALL_PATH)/mcu_sdk
TIOVX_PATH          ?=$(PSDK_INSTALL_PATH)/tiovx
PLATFORM_PATH       ?=$(PSDK_INSTALL_PATH)/platform

LINUX_FS_PATH       ?=$(PSDK_INSTALL_PATH)/targetfs
TVM_HOME            ?=$(PSDK_INSTALL_PATH)/targetfs/usr/include/tvm
TF_REPO_PATH        ?=$(PSDK_INSTALL_PATH)/targetfs/usr/include/tensorflow
ONNX_REPO_PATH      ?=$(PSDK_INSTALL_PATH)/targetfs/usr/include/onnxruntime

# Below are only needed for PC emulation Test bench build
TIDL_OPENCV_PATH    ?="$(PSDK_INSTALL_PATH)/opencv-4.1.0"
TIDL_PROTOBUF_PATH  ?= $(PSDK_INSTALL_PATH)/protobuf-3.21.12
TIDL_FLATBUF_PATH   ?="$(PSDK_INSTALL_PATH)/flatbuffers-1.12.0"
TIDL_GRAPHVIZ_PATH  ?="/usr"



# Location of the CUDA Toolkit
CUDA_PATH ?= /usr/local/cuda
endif

SHOW_COMMANDS       ?= 0

# Default CORE is dsp
# Supported values: dsp, eve, arm
CORE ?= dsp

# Default RTOS is FREERTOS
# Supported values: FREERTOS, SYSBIOS
RTOS ?= FREERTOS

# Default C66
# Supported C64T, C64P, C64, C66, C674, C67, C67P, m4
TARGET_CPU ?= C71

#C7x Version (C7120/C7100/C7504)
#Compiles code for a particular C7x mode
C7x_HOSTEMU_COMPILER_STRING:=
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721E j721e))
    TARGET_C7X_VERSION ?= C7100
    SI_VER = 7100
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721S2 j721s2 J784S4 j784s4 J742S2 j742s2))
    TARGET_C7X_VERSION ?= C7120
    SI_VER = 7120
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
    TARGET_C7X_VERSION ?= C7504
    SI_VER = 7504
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
    TARGET_C7X_VERSION ?= C7524
    SI_VER = 7524
    C7x_HOSTEMU_COMPILER_STRING:=-MMA2_256
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
    TARGET_C7X_VERSION ?= C7604
    SI_VER = 7604
    C7x_HOSTEMU_COMPILER_STRING:=-MMA3_1024
endif

ifeq ($(RTOS_SDK),mcu_sdk)
  DMA_UTILS_PATH      ?=$(PSDK_INSTALL_PATH)/dmautils
  PDK_INSTALL_PATH    = .
else ifeq ($(RTOS_SDK),mcu_plus_sdk)
  DMA_UTILS_PATH      ?=$(MCU_PLUS_SDK_PATH)/source/drivers/dmautils
  PDK_INSTALL_PATH    = .
else
  DMA_UTILS_PATH      ?=$(PDK_INSTALL_PATH)/ti/drv/udma/dmautils
  MCU_PLUS_SDK_PATH   = .
  MCU_SDK_PATH        = .
endif

#Temporary flag to use older library paths
#TIDL_BUILD_PATHS = LEGACY

ifeq ($(CORE),eve)
TARGET_CPU:=
endif

# Supported values: debug and release)
TARGET_BUILD ?= release

FINAL_RELEASE_BUILD ?= 1
# Set this to enable
TIDL_PRESILICON_SDK_UC_ENABLE?=0
# Set this if per frame performance information is required to be stored
TIDL_ENABLE_PER_FRAME_PERFORMANCE?=0

TIDL_WRITE_PRINTS_TO_BUF ?= 0

ifeq ($(TARGET_PLATFORM) ,PC)
TIDL_PRESILICON_SDK_UC_ENABLE=0
TIDL_WRITE_PRINTS_TO_BUF = 0
BUILD_WITH_OPENCV    ?= 1
BUILD_WITH_STATIC_ARRAYS ?= 0
COMPARE_SINGLE_LAYER ?= 0
USE_HOST_FILE_IO ?=0
USE_HOST_PRINTF ?=0
else
BUILD_WITH_OPENCV    ?= 0
BUILD_WITH_STATIC_ARRAYS ?= 0
COMPARE_SINGLE_LAYER ?= 0
ifeq ($(TIDL_BUILD_FOR_LOKI), 1)
USE_HOST_FILE_IO ?=0
else
USE_HOST_FILE_IO ?=1
endif
USE_HOST_PRINTF ?=0
endif

# Default C7X_CORE_ID used by TIDL is 1 for J7ES and J7AEP, can be 1,2,3,4 for J7AHP
C7X_CORE_ID ?=1
