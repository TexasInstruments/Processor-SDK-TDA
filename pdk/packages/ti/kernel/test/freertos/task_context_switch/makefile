#
# This file is the makefile for building FreeRTOS Task Switch Example App
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif


APP_NAME = freertos_test_task_context_switch
BUILD_OS_TYPE=freertos
CFLAGS_TEST_DEFINES =

SOC_DIR=$(SOC)
ifeq ($(SOC), j742s2)
  SOC_DIR=j784s4
endif
SRCDIR = .
INCDIR = ../
INCDIR += ${FREERTOS_KERNEL_INSTALL_PATH}/FreeRTOS-Kernel/include
INCDIR += $(PDK_FREERTOS_COMP_PATH)/freertos/portable/TI_CGT/$(ISA)
INCDIR += $(PDK_CSL_COMP_PATH)/arch/$(ISA)
INCDIR += $(PDK_FREERTOS_COMP_PATH)/freertos/config/$(SOC_DIR)/$(ISA)
INCDIR += $(PDK_FREERTOS_COMP_PATH)/freertos/dpl/common

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk
ifeq ($(BUILD_OS_TYPE), freertos)
COMP_LIST_COMMON = $(PDK_COMMON_FREERTOS_COMP)
else
# List all the components required by the application
endif

ifeq ($(ISA),$(filter $(ISA), c66))
  EXTERNAL_LNKCMD_FILE_LOCAL = $(PDK_INSTALL_PATH)/ti/build/$(SOC)/linker_c66_freertos.cmd
endif
ifeq ($(ISA),$(filter $(ISA), r5f))
ifeq ($(SOC),$(filter $(SOC), tpr12 awr294x))
  EXTERNAL_LNKCMD_FILE_LOCAL = $(PDK_INSTALL_PATH)/ti/build/$(SOC)/linker_$(CORE)_freertos.lds
else
  EXTERNAL_LNKCMD_FILE_LOCAL = $(PDK_INSTALL_PATH)/ti/build/$(SOC)/linker_r5_freertos.lds
endif
endif

ifeq ($(SOC),$(filter $(SOC), tpr12))
# Append local auxillary linker command file
APPEND_LNKCMD_FILE = ../build/$(SOC)/linker_$(ISA).cmd
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . ../FREERTOS_log.h
SRCS_COMMON += main.c task_context_switch.c

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(CFLAGS_TEST_DEFINES)

# Core/SoC/platform specific source files and CFLAGS
# Example:
#   SRCS_<core/SoC/platform-name> =
#   CFLAGS_LOCAL_<core/SoC/platform-name> =

# Include common make files
ifeq ($(MAKERULEDIR), )
#Makerule path not defined, define this and assume relative path from ROOTDIR
  MAKERULEDIR := $(ROOTDIR)/ti/build/makerules
  export MAKERULEDIR
endif
include $(MAKERULEDIR)/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
