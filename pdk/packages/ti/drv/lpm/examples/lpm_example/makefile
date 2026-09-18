#
# This file is common makefile for building LPM example for both TI-RTOS/baremetal
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

APP_NAME = lpm_example_$(BUILD_OS_TYPE)

SRCDIR = src
INCDIR = 

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

ifeq ($(BUILD_OS_TYPE), freertos)
  COMP_LIST_COMMON = $(PDK_COMMON_FREERTOS_COMP)
  INCLUDE_EXTERNAL_INTERFACES += freertos
  SRCS_COMMON = main_rtos.c overrides/$(SOC)/$(CORE)/r5_mpu_freertos.c
  EXTERNAL_LNKCMD_FILE_LOCAL = overrides/$(SOC)/$(CORE)/linker_r5_freertos.lds
endif

# List all the specific components required by the application
COMP_LIST_COMMON += sciserver_tirtos ipc sbl_lib_cust spi_dma lpm

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = .
SRCS_COMMON += ../../../lpm/src/lpm_uart_drv.c

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS)

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
