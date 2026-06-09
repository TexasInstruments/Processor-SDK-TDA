#
# This file is the makefile for building UDMA DRU8 test app.
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), baremetal)
    APP_NAME = dru8_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = src
INCDIR += $(PDK_INSTALL_PATH)/ti/drv/udma/unit_test/udma_ut/src/soc/$(SOC) 
INCDIR += $(PDK_INSTALL_PATH)/ti/csl/soc/$(SOC)/src/

# List all the external components/interfaces, whose interface header files
# need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk udma

ifeq ($(BUILD_OS_TYPE), baremetal)
  # List all the components required by the application
  COMP_LIST_COMMON += $(PDK_COMMON_BAREMETAL_COMP) udma_apputils
  SRCS_COMMON      += main_baremetal.c
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src
CFLAGS_LOCAL_COMMON += $(PDK_CFLAGS)

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
