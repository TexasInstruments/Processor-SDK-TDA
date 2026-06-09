#
# Makefile for building saul6 ip fma app
#

ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), baremetal)
  APP_NAME = sms_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . ./src $(PDK_INSTALL_PATH)/ti/ip_fma/src
INCDIR = . $(PDK_INSTALL_PATH)/ti/ip_fma/inc

# List all the external components/interfaces, whose interface header files
# need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

ifeq ($(BUILD_OS_TYPE), baremetal)
  COMP_LIST_COMMON += $(PDK_COMMON_BAREMETAL_COMP)
  SRCS_COMMON += main_baremetal.c
endif

# List all the specific components required by the application
COMP_LIST_COMMON += ip_fma

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src
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
