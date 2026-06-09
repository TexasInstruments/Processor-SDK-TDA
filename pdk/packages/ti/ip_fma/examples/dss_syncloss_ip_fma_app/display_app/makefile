#
# This file is the makefile for building display app for dss8 ip fma test
# and baremetal
#

ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), freertos)
    APP_NAME = dss8_display_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . src
INCDIR = . src

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk


ifeq ($(BUILD_OS_TYPE),freertos)
  CFLAGS_OS_DEFINES = -DFREERTOS
  INCLUDE_EXTERNAL_INTERFACES += freertos
  COMP_LIST_COMMON += $(PDK_COMMON_FREERTOS_COMP) dss_app_utils
  SRCS_COMMON += dss_main_rtos.c dss_display_test.c
endif

# List all the specific components required by the application
COMP_LIST_COMMON += fvid2 dss

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src
CFLAGS_LOCAL_COMMON += $(PDK_CFLAGS) $(DSS_CFLAGS) $(CFLAGS_OS_DEFINES)

# Path relative to this makefile or project
EXTERNAL_LNKCMD_FILE_LOCAL = linker_r5_freertos_mcu2_0_dss.lds

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

