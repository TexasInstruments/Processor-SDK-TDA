#
# This file is the makefile for building dru11 dru12 ip fma test
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), freertos)
    APP_NAME = dru11_dru12_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . src
INCDIR = . inc

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

# List all the components required by the application
ifeq ($(BUILD_OS_TYPE), freertos)
  COMP_LIST_COMMON = $(PDK_COMMON_FREERTOS_COMP) udma_apputils
  INCLUDE_EXTERNAL_INTERFACES += freertos
  SRCS_COMMON = main_rtos.c
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src
SRCS_COMMON += dru11_dru12_test.c
CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(DRU11_DRU12_CFLAGS) $(CFLAGS_OS_DEFINES)

# Append a local linker command file 
APPEND_LNKCMD_FILE = dru_buffers_sections.cmd

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
