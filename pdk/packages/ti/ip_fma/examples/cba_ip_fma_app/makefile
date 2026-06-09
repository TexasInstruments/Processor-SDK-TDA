#
# This file is the makefile for building CBA Periodic Readback test app
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), baremetal)
    APP_NAME = cba_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . src $(PDK_INSTALL_PATH)/ti/ip_fma/src
INCDIR = . $(PDK_INSTALL_PATH)/ti/ip_fma/inc
INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient


# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

# List all the components required by the application
COMP_LIST_COMMON = ip_fma

ifeq ($(BUILD_OS_TYPE), baremetal)
  COMP_LIST_COMMON += $(PDK_COMMON_BAREMETAL_COMP)
  SRCS_COMMON += main_baremetal.c
  SRCS_COMMON += cbass_test_qos.c
  SRCS_COMMON += cbass_test_err.c
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src
CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(CBA_CFLAGS) $(CFLAGS_OS_DEFINES)

#Set compiler flag to enable fault-injection scenario
ifeq ($(FAULT_INJECTION), true)
  CFLAGS_LOCAL_COMMON += -DFAULT_INJECTION_ENABLED
endif

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