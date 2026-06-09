ifeq ($(OS),Windows_NT)
  HOST_OS=Windows_NT
else
  OS=$(shell uname -s)
  ifeq ($(OS),Linux)
    HOST_OS=LINUX
    HOST_NUM_CORES := $(shell cat /proc/cpuinfo | grep processor | wc -l)
  endif
endif


ifeq ($(HOST_OS),CYGWIN)
  PSDK_PATH ?= $(shell cygpath -m ${abspath ..})
else
  PSDK_PATH ?= $(abspath ..)
endif

ifeq ($(HOST_OS),CYGWIN)
  ETHFW_PATH ?= $(shell cygpath -m ${abspath .})
else
  ETHFW_PATH ?= $(abspath .)
endif

PSDK_TOOLS_PATH ?= $(PSDK_PATH)

# SafeRTOS version
SAFERTOS_VERSION_r5f_j721e = 009-005-199-024-219-005
SAFERTOS_VERSION_r5f_j7200 = 009-005-199-024-219-005
SAFERTOS_VERSION_r5f_j784s4 = 009-005-199-024-219-005
SAFERTOS_KERNEL_INSTALL_PATH_r5f_J721E ?= $(PSDK_PATH)/safertos_j721e_r5f_$(SAFERTOS_VERSION_r5f_j721e)
SAFERTOS_KERNEL_INSTALL_PATH_r5f_J7200 ?= $(PSDK_PATH)/safertos_j7200_r5f_$(SAFERTOS_VERSION_r5f_j7200)
SAFERTOS_KERNEL_INSTALL_PATH_r5f_J784S4 ?= $(PSDK_PATH)/safertos_j784s4_r5f_$(SAFERTOS_VERSION_r5f_j784s4)
SAFERTOS_ISA_EXT_r5f = 199_TI_CR5
SAFERTOS_COMPILER_EXT_r5f = 024_Clang

#CCS Path needed for CCS project build
CCS_PATH ?= D:/ccs_v8_3/ccsv8
TIARMCGT_ROOT = $(PSDK_TOOLS_PATH)/ti-cgt-arm_20.2.0.LTS
TIARMCGT_LLVM_ROOT = $(PSDK_TOOLS_PATH)/ti-cgt-armllvm_4.0.4.LTS

REMOTE_DEVICE_PATH ?= $(PSDK_PATH)/remote_device

PDK_PATH ?= $(PSDK_PATH)/pdk

BUILD_OS ?= Linux

ifeq ($(BUILD_OS),Linux)
  GCC_LINUX_ROOT ?= /usr/
endif
