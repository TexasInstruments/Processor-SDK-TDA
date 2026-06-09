#
# This file is the makefile for building udma1 udma2 ip fma tests
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), baremetal)
  APP_NAME = udma1_udma2_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . src
INCDIR = . src

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

ifeq ($(BUILD_OS_TYPE), baremetal)
  COMP_LIST_COMMON = $(PDK_COMMON_BAREMETAL_COMP) udma_apputils
  SRCS_COMMON = main_baremetal.c
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = . src/
SRCS_COMMON += udma1_udma2_test.c

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(UDMA1_UDMA2_CFLAGS) $(CFLAGS_OS_DEFINES)

# Append a local linker command file 
APPEND_LNKCMD_FILE = udma_buffers_sections.cmd

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
