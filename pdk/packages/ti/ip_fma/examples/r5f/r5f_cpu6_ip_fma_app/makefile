#
# Makefile for building r5f ip fma app
#

ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), baremetal)
  APP_NAME = r5f_cpu6_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . ./src $(PDK_INSTALL_PATH)/ti/ip_fma/src $(PDK_INSTALL_PATH)/ti/ip_fma/examples/r5f/common/src
INCDIR = . $(PDK_INSTALL_PATH)/ti/ip_fma/inc $(PDK_INSTALL_PATH)/ti/ip_fma/examples/r5f/common/inc/

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

ifeq ($(BUILD_OS_TYPE), baremetal)
  COMP_LIST_COMMON += $(PDK_COMMON_BAREMETAL_COMP)
  INCLUDE_EXTERNAL_INTERFACES += baremetal
  SRCS_COMMON += main_baremetal.c
  SRCS_COMMON += ip_fma_r5f_app_utils.c
endif

# List all the specific components required by the application
COMP_LIST_COMMON += ip_fma

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . ../common

CFLAGS_LOCAL_COMMON += $(PDK_CFLAGS) $(CFLAGS_OS_DEFINES)

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
