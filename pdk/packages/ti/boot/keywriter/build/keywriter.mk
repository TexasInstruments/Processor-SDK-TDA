#
# This file is the makefile for building Keywriter image
# that is loaded by R5 ROM.
#
include $(PDK_INSTALL_PATH)/ti/build/Rules.make

ifeq ($(SBL_IMAGE_TYPE), combined)
  APP_NAME                     = keywriter_img_combined
  LOCAL_APP_NAME               = keywriter_img_combined_$(SOC)
else
  APP_NAME                     = keywriter_img
  LOCAL_APP_NAME               = keywriter_img_$(SOC)
endif
BUILD_OS_TYPE                = baremetal

# J784S4 EVM does not use PMIC
ifneq ($(filter $(BOARD),j784s4_evm j742s2_evm),)
VPP_EN_CONTROL               = gpio
else
VPP_EN_CONTROL               = pmic
endif

KEYWRITER_APP_DIR            = $(PDK_KEYWR_COMP_PATH)
SRCDIR                      += $(KEYWRITER_APP_DIR)

INCDIR                      += $(PDK_INSTALL_PATH)/ti/board/src/$(BOARD)/include
INCDIR                      += $(PDK_INSTALL_PATH)/ti/csl
INCDIR                      += $(KEYWRITER_APP_DIR)/boardcfgs/$(SOC)
ifeq ($(VPP_EN_CONTROL),pmic)
INCDIR                      += $(PDK_INSTALL_PATH)/ti/drv/pmic
INCDIR                      += $(PDK_INSTALL_PATH)/ti/drv/pmic/include
INCDIR                      += $(PDK_INSTALL_PATH)/ti/drv/pmic/test/common
INCDIR                      += $(PDK_INSTALL_PATH)/ti/drv/pmic/test/power_test
endif
INCDIR                      += $(KEYWRITER_APP_DIR)/soc/$(SOC)

PACKAGE_SRCS_COMMON          = . ../boardcfgs/$(SOC) ../scripts ../soc/common ../soc/$(SOC)
PACKAGE_SRCS_COMMON         += ../tifs_bin/$(SOC) ../x509cert ../init.asm ../main.c
PACKAGE_SRCS_COMMON         += ../keywriter_component.mk ../main.h

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES  = pdk

# List all the components required by the application
COMP_LIST_COMMON             = sciclient_direct board uart osal_nonos csl csl_init i2c gpio rm_pm_hal
ifeq ($(SOC),$(filter $(SOC), j7200 j784s4 j742s2 j721s2))
COMP_LIST_COMMON += lpm_stub
endif

ifeq ($(VPP_EN_CONTROL),pmic)
COMP_LIST_COMMON            += pmic
endif

CFLAGS_LOCAL_COMMON          = $(PDK_CFLAGS)

ifeq ($(SOC),j721e)
  # J721E EVM reference on ti.com shows use of LDO3 to set VPP. However, as LDO3
  # is used for many other generic power sequences, it is desirable to have
  # dedicated source for temporary VPP supply (VPP for efuse is not intended to
  # remain asserted persistently).
  #
  # Recommendation is to use another GPIO to drive an external LDO - see
  # https://www.ti.com/lit/ug/slvuc99a/slvuc99a.pdf for more details. The
  # default keywriter example code uses this method. Comment this flag only if
  # testing keywriter on a TI J721E EVM and you need to use PMICB LDO3 to toggle
  # VPP for efuse programming.

  CFLAGS_LOCAL_COMMON		+= -DJ721E_USE_GPIO_FOR_VPP
endif

# Combined boot image flags
ifeq ($(SBL_IMAGE_TYPE), combined)
  SBL_CFLAGS += -DSBL_COMBINED_BOOT
  CFLAGS_LOCAL_COMMON += -DSBL_COMBINED_BOOT
endif

SRCS_COMMON                 += main.c
ifeq ($(VPP_EN_CONTROL),pmic)
SRCS_COMMON                 += soc/common/pmic_example.c
endif
SRCS_COMMON                 += soc/$(SOC)/keywriter_utils.c
SRCS_ASM_COMMON             += init.asm

EXTERNAL_LNKCMD_FILE_LOCAL   = $(PDK_KEYWR_COMP_PATH)/soc/common/linker.lds

export R5_STARTUP_MODE      := SPLIT_MODE

SCI_CLIENT_DIR              := $(PDK_INSTALL_PATH)/ti/drv/sciclient
SCI_CLIENT_TOOLS_BIN2C_DIR  := $(SCI_CLIENT_DIR)/tools/bin2c

ifeq ($(OS),Windows_NT)
  BIN2C_EXE                 := $(SCI_CLIENT_TOOLS_BIN2C_DIR)/bin2c.exe
else
  BIN2C_EXE                 := $(SCI_CLIENT_TOOLS_BIN2C_DIR)/bin2c.out
endif

# signed keywr sys bin
SIGNED_SYSFW_BIN            := $(KEYWRITER_APP_DIR)/tifs_bin/$(SOC)/ti-fs-keywriter.bin

# Include common make files
ifeq ($(MAKERULEDIR), )
  MAKERULEDIR               := $(ROOTDIR)/ti/build/makerules
  export MAKERULEDIR
endif
include $(MAKERULEDIR)/platform.mk
include $(MAKERULEDIR)/common.mk
# Uncomment the below to auto-generate new "tifs_keywriter.h" 
#include $(PDK_KEYWR_COMP_PATH)/build/bin2c.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
