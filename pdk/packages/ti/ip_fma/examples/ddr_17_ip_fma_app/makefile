#
# This file is the makefile for building ddr ip fma tests
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), baremetal)
    APP_NAME = ddr_17_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . src $(PDK_INSTALL_PATH)/ti/ip_fma/src
INCDIR = . inc $(PDK_INSTALL_PATH)/ti/ip_fma/inc $(PDK_INSTALL_PATH)/ti/board

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk csl_arch

# List all the components required by the application
ifeq ($(BUILD_OS_TYPE), baremetal)
  COMP_LIST_COMMON += $(PDK_COMMON_BAREMETAL_COMP) ip_fma
  SRCS_COMMON += main_baremetal.c
  SRCS_COMMON += ddr_parity_protection.c
  SRCS_COMMON += esm.c
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src
CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(DDR_CFLAGS) $(CFLAGS_OS_DEFINES) -O0

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