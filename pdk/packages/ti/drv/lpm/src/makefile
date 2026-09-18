#
# This file is the makefile for building LPM library.
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

MODULE_NAME = lpm

SRCDIR  = src
INCDIR =
#INCDIR = include soc/$(SOC)
INCDIR += $(PDK_SBL_COMP_PATH)/soc/k3

# Only SOCs with soc-specific boot core definitions have a soc/ subdirectory
lpm_LIB_SOC_LIST = j721e j7200 j784s4
ifneq ($(filter $(SOC), $(lpm_LIB_SOC_LIST)),)
SRCDIR += soc/$(SOC)
endif
INCDIR += $(PDK_INSTALL_PATH)/ti/drv/pmic/include
INCDIR += $(PDK_INSTALL_PATH)/ti/build/unit-test/config

# List all the external components/interfaces, whose interface header files
# need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk
INCLUDE_INTERNAL_INTERFACES = board sbl_lib_cust spi_dma pmic ipc
COMP_LIST_COMMON = 

PACKAGE_SRCS_COMMON = lpm.h lpm_component.mk makefile .gitignore docs $(SRCDIR)
PACKAGE_SRCS_COMMON += include/lpm_boot.h
PACKAGE_SRCS_COMMON += include/lpm_pmic.h
PACKAGE_SRCS_COMMON += include/lpm_ipc.h
PACKAGE_SRCS_COMMON += include/lpm_mmr_functions.h
PACKAGE_SRCS_COMMON += include/io_retention/cgt_pragmas.h
PACKAGE_SRCS_COMMON += include/io_retention/dev_info.h
PACKAGE_SRCS_COMMON += include/io_retention/dmsc_cm.h
PACKAGE_SRCS_COMMON += src/lpm_uart_baremetal.h
PACKAGE_SRCS_COMMON += src/lpm_uart_drv.h
ifneq ($(filter $(SOC), $(lpm_LIB_SOC_LIST)),)
PACKAGE_SRCS_COMMON += soc/$(SOC)/boot_core_defs.h
endif
PACKAGE_SRCS_COMMON += config_mk.bld package.bld package.xdc package.xs
PACKAGE_SRCS_COMMON += Settings.xdc.xdt lpmver.h lpmver.h.xdt

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON  = lpm_boot.c
SRCS_COMMON += lpm_mcu_only_recovery.c
SRCS_COMMON += lpm_ipc.c
SRCS_COMMON += lpm_uart_drv.c
SRCS_COMMON += lpm_mmr_functions.c
ifneq ($(filter $(SOC), $(lpm_LIB_SOC_LIST)),)
SRCS_COMMON += soc/$(SOC)/boot_core_defs.c
endif
SRCS_COMMON += lpm_pmic_i2c_drv.c

# To enable debug halt loops in mcu_only and io_retention examples, add -DLPM_DEBUG to CFLAGS_LOCAL_COMMON
CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) -DUNITY_INCLUDE_CONFIG_V2_H -DLINUX_OS -DA72_LINUX_OS -DA72_LINUX_OS_IPC_ATTACH -DSBL_LOG_LEVEL=3 -DMPU1_HLOS_BOOT_ENABLED

ifeq ($(SOC), j784s4)
CFLAGS_LOCAL_COMMON += -DENABLE_DMA
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
