#
# This file is the makefile for building c7x_mma_error injection code
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

APP_NAME = c7x_illegal_instruction_testapp_$(BUILD_OS_TYPE)

SRCDIR = .
INCDIR = 

# Include PDK components
INCLUDE_EXTERNAL_INTERFACES = pdk

# Include RTOS components as per requirement
ifeq ($(BUILD_OS_TYPE), freertos)
  COMP_LIST_COMMON = $(PDK_COMMON_FREERTOS_COMP)
  INCLUDE_EXTERNAL_INTERFACES += freertos
  SRCS_COMMON = c7x_illegal_instruction_main.c c7x_illegal_instructions.c
  CONFIG_BLD_LNK_c7x = $(IP_FMA_COMP_PATH)/examples/c7x_illegal_instruction/linker_c7x_freertos.lds
endif

# Add include directories
INCDIR += . $(PDK_INSTALL_PATH)/ti/csl/

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = .

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(CFLAGS_OS_DEFINES)

# Warning Suppression:
# error #1-D: last line of file ends without a newline
# error #770-D: conversion from pointer to smaller integer
CFLAGS_LOCAL_COMMON += --diag_suppress=1 --diag_suppress=770
LNKFLAGS_LOCAL_c7x_1 += --diag_suppress=1 --diag_suppress=770

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
