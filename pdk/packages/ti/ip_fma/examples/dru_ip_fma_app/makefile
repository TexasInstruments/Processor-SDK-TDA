#
# This file is the makefile for building UDMA DRU Periodic Readback test app
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

ifeq ($(BUILD_OS_TYPE), freertos)
    APP_NAME = dru_ip_fma_app_$(BUILD_OS_TYPE)
endif

SRCDIR = . src $(IP_FMA_COMP_PATH)/src
INCDIR = . $(IP_FMA_COMP_PATH)/inc

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

# List all the components required by the application
COMP_LIST_COMMON = ip_fma udma_apputils

ifeq ($(BUILD_OS_TYPE), freertos)
  COMP_LIST_COMMON += $(PDK_COMMON_FREERTOS_COMP)
  INCLUDE_EXTERNAL_INTERFACES += freertos
endif

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = .
SRCS_COMMON = main_rtos.c dru_periodic_readback.c
CFLAGS_LOCAL_COMMON += $(PDK_CFLAGS) $(DRU_CFLAGS) $(CFLAGS_OS_DEFINES)

# Include common make files
ifeq ($(MAKERULEDIR), )
#Makerule path not defined, define this and assume relative path from ROOTDIR
  MAKERULEDIR := $(ROOTDIR)/ti/build/makerules
  export MAKERULEDIR
endif
include $(MAKERULEDIR)/common.mk
