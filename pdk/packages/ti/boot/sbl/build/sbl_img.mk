#
# This file is the makefile for building SBL image
# that is loaded by R5 ROM.
#
include $(PDK_INSTALL_PATH)/ti/build/Rules.make

HLOS_SUFFIX=
ifeq ($(HLOS_BOOT),yes)
HLOS_SUFFIX=_hlos
endif
COMBINE_SUFFIX=
ifeq ($(SBL_IMAGE_TYPE),combined)
COMBINE_SUFFIX=_combined
endif
HS_SUFFIX=
ifeq ($(BUILD_HS),yes)
HS_SUFFIX=_hs
endif
HS_FS_SUFFIX=
ifeq ($(BUILD_HS_FS),yes)
  HS_FS_SUFFIX=_hs_fs
endif
DMA_SUFFIX=
ifeq ($(BOOTMODE), $(filter $(BOOTMODE),ospi cust xip))
  ifeq ($(SBL_USE_DMA),no)
    DMA_SUFFIX=_nondma
  endif
endif
EMMC_SUFFIX=
OSPI_NAND_SUFFIX=
ifeq ($(OSPI_NAND), yes)
  OSPI_NAND_SUFFIX=_nand
endif
SECURE_HSM_BOOT_SUFFIX=
ifeq ($(BOOTMODE), $(filter $(BOOTMODE),uart ospi))
  ifeq ($(SECURE_HSM_BOOT), yes)
    SECURE_HSM_BOOT_SUFFIX=hsm_boot_
  endif
endif

# if you want to boot app more than 500KB you need to change the following macro
# for instance your app is x KB then you need to give the macro with the following value from cmd line
# (x * 1024) in hexadecimal
ifeq ($(HLOS_BOOT), yes)
# HLOS Image size - 24 MB
MAX_APP_SIZE_EMMC ?= 0x1800000
else
MAX_APP_SIZE_EMMC ?= 0x7D000
endif

# If no address is specified from command prompt, SBL copies EEPROM content to 0x90000000 
EEPROM_DATA_DDR_ADDRESS ?= 0x90000000

ifeq ($(BOOT_PERF), yes) 
  ifeq ($(SBL_IMAGE_TYPE),combined)
    APP_NAME = sbl_boot_perf_$(BOOTMODE)_img_combined$(HS_SUFFIX)
    LOCAL_APP_NAME = sbl_boot_perf_$(BOOTMODE)_img_combined$(HS_SUFFIX)_$(CORE)
  else
    APP_NAME = sbl_boot_perf_$(BOOTMODE)$(OSPI_NAND_SUFFIX)_img$(HS_SUFFIX)
    LOCAL_APP_NAME = sbl_boot_perf_$(BOOTMODE)$(OSPI_NAND_SUFFIX)_img$(HS_SUFFIX)_$(CORE)
  endif
else ifeq ($(BOOTMODE), xip)
  ifeq ($(OSPI_FREQ), 133)
    APP_NAME = sbl_xip_133_img$(HS_SUFFIX)
    LOCAL_APP_NAME = sbl_xip_133_img_$(CORE)
  else
    APP_NAME = sbl_xip_img$(HS_SUFFIX)
    LOCAL_APP_NAME = sbl_xip_img_$(CORE)
  endif
else ifeq ($(RAT), 1)
  APP_NAME = sbl_cust_rat_main_ocm_img
  LOCAL_APP_NAME = sbl_cust_rat_main_ocm_img_$(CORE)
else ifeq ($(SBL_ENABLE_BIST), yes)
  APP_NAME = sbl_$(BOOTMODE)_bist_img$(HS_SUFFIX)$(HS_FS_SUFFIX)
  LOCAL_APP_NAME=sbl_$(BOOTMODE)_bist_img_$(CORE)
else
  APP_NAME = sbl_$(SECURE_HSM_BOOT_SUFFIX)$(BOOTMODE)$(OSPI_NAND_SUFFIX)$(EMMC_SUFFIX)_img$(COMBINE_SUFFIX)$(HLOS_SUFFIX)$(HS_SUFFIX)$(HS_FS_SUFFIX)
  LOCAL_APP_NAME=sbl_$(SECURE_HSM_BOOT_SUFFIX)$(BOOTMODE)$(OSPI_NAND_SUFFIX)$(EMMC_SUFFIX)_img$(COMBINE_SUFFIX)$(HLOS_SUFFIX)_$(CORE)
endif
BUILD_OS_TYPE = baremetal

SRCDIR += $(PDK_SBL_COMP_PATH)/board/k3

INCDIR      += $(PDK_SBL_COMP_PATH)
INCDIR      += $(PDK_SBL_COMP_PATH)/soc
INCDIR      += $(PDK_SBL_COMP_PATH)/soc/k3

SOC_DIR=$(SOC)
ifeq ($(SOC), j742s2)
  SOC_DIR=j784s4
endif

# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

# List all the components required by the application
COMP_LIST_COMMON = $(PDK_COMMON_BAREMETAL_COMP)
ifneq ($(strip $(HS_SUFFIX)),) #if $(HS_SUFFIX) is non-empty
  SCICLIENT := $(filter sciclient%,$(COMP_LIST_COMMON))
  COMP_LIST_COMMON := $(subst $(SCICLIENT),$(SCICLIENT)_hs,$(COMP_LIST_COMMON))
endif

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) $(SBL_CFLAGS)

# Check for custom flags
ifeq ($(BOOTMODE), cust)
  SBL_CFLAGS = $(CUST_SBL_FLAGS)
  ifeq ($(BOOT_PERF), yes)
    COMP_LIST_COMMON += sbl_boot_perf_lib_$(BOOTMODE)$(DMA_SUFFIX)$(HS_SUFFIX)
  else  
    COMP_LIST_COMMON += sbl_lib_$(BOOTMODE)$(DMA_SUFFIX)$(HS_SUFFIX)
  endif
  ifeq ($(RAT), 1)
    SBL_CFLAGS += -DSBL_OCM_MAIN_DOMAIN_RAT
  endif
  ifeq ($(OSPI_NAND), yes)
    SBL_CFLAGS += -DOSPI_NAND_BOOT
  endif
else ifeq ($(BOOTMODE), xip)
  SBL_CFLAGS = $(CUST_SBL_FLAGS)
  SBL_CFLAGS += -DBUILD_XIP
  ifeq ($(OSPI_FREQ), 133)
    SBL_CFLAGS += -DOSPI_FREQ_133
  else
    SBL_CFLAGS += -DOSPI_FREQ_166
  endif
  COMP_LIST_COMMON += sbl_lib_cust$(DMA_SUFFIX)$(HS_SUFFIX)
else ifeq ($(BOOTMODE), $(filter $(BOOTMODE), emmc_uda emmc_boot0))
  COMP_LIST_COMMON += sbl_lib_emmc$(DMA_SUFFIX)$(HLOS_SUFFIX)$(HS_SUFFIX)
else
  COMP_LIST_COMMON += sbl_lib_$(BOOTMODE)$(DMA_SUFFIX)$(HLOS_SUFFIX)$(HS_SUFFIX)
endif # ifeq ($(BOOTMODE), cust)

ifeq ($(SBL_USE_DMA),yes)
  SBL_CFLAGS += -DSBL_USE_DMA=1
else
  SBL_CFLAGS += -DSBL_USE_DMA=0
endif

ifeq ($(BOOT_PERF), yes)
  SBL_CFLAGS += -DBOOT_PERF 
endif

# Combined boot image flags
ifeq ($(SBL_IMAGE_TYPE), combined)
  SBL_CFLAGS += -DSBL_COMBINED_BOOT
endif

# HLOS Boot flags
ifeq ($(HLOS_BOOT),yes)
  SBL_CFLAGS += -DSBL_ENABLE_HLOS_BOOT -DSBL_HLOS_OWNS_FLASH
endif
ifeq ($(BUILD_HS),yes)
  SBL_CFLAGS += -DSBL_BUILD_HS -DBUILD_HS
endif
# BOOTMODE specific CFLAGS
ifeq ($(BOOTMODE), mmcsd)
  SBL_CFLAGS += -DBOOT_MMCSD
endif # ifeq ($(BOOTMODE), mmcsd)

ifeq ($(BOOTMODE), emmc_uda)
  SBL_CFLAGS += -DBOOT_EMMC_UDA
else ifeq ($(BOOTMODE), emmc_boot0)
  SBL_CFLAGS += -DBOOT_EMMC_BOOT0
endif # ifeq ($(BOOTMODE), emmc_uda)

ifeq ($(BOOTMODE), ospi)
  SBL_CFLAGS += -DBOOT_OSPI
  ifeq ($(OSPI_NAND), yes)
    SBL_CFLAGS += -DOSPI_NAND_BOOT
  endif
endif # ifeq ($(BOOTMODE), ospi)

ifeq ($(SECURE_HSM_BOOT), yes)
  SBL_CFLAGS += -DSECURE_HSM_BOOT
endif

ifeq ($(BOOTMODE), hyperflash)
  SBL_CFLAGS += -DBOOT_HYPERFLASH
endif # ifeq ($(BOOTMODE), hyperflash)

ifeq ($(BOOTMODE), uart)
  SBL_CFLAGS += -DBOOT_UART
endif # ifeq ($(BOOTMODE), uart)

ifeq ($(BOOTMODE), qspi)
  SBL_CFLAGS += -DBOOT_QSPI
endif # ifeq ($(BOOTMODE), qspi)

ifeq ($(filter $(SBL_CFLAGS), -DBOOT_MMCSD), -DBOOT_MMCSD)
  COMP_LIST_COMMON += mmcsd fatfs_indp
endif # ifeq ($(filter $(SBL_CFLAGS), -DBOOT_MMCSD), -DBOOT_MMCSD)

ifeq ($(filter $(SBL_CFLAGS), -DBOOT_EMMC_UDA), -DBOOT_EMMC_UDA)
  COMP_LIST_COMMON += mmcsd fatfs_indp
endif # ifeq ($(filter $(SBL_CFLAGS), -DBOOT_EMMC_UDA), -DBOOT_EMMC_UDA)

ifeq ($(filter $(SBL_CFLAGS), -DBOOT_EMMC_BOOT0), -DBOOT_EMMC_BOOT0)
  COMP_LIST_COMMON += mmcsd fatfs_indp
endif # ifeq ($(filter $(SBL_CFLAGS), -DBOOT_EMMC_BOOT0), -DBOOT_EMMC_BOOT0)

ifeq ($(filter $(SBL_CFLAGS), -DBOOT_HYPERFLASH), -DBOOT_HYPERFLASH)
  COMP_LIST_COMMON += spi
endif # ifeq ($(filter $(SBL_CFLAGS), -DBOOT_HYPERFLASH), -DBOOT_HYPERFLASH)

ifeq ($(filter $(SBL_CFLAGS), -DBOOT_OSPI), -DBOOT_OSPI)
ifeq ($(filter $(SBL_CFLAGS), -DSBL_USE_DMA=1), -DSBL_USE_DMA=1)
  COMP_LIST_COMMON += spi_dma
else
  COMP_LIST_COMMON += spi
endif # ifeq ($(filter $(SBL_CFLAGS), -DSBL_USE_DMA=1), -DSBL_USE_DMA=1)
endif # ifeq ($(filter $(SBL_CFLAGS), -DBOOT_OSPI), -DBOOT_OSPI)

ifeq ($(filter $(SBL_CFLAGS), -DBOOT_QSPI), -DBOOT_QSPI)
ifeq ($(filter $(SBL_CFLAGS), -DSBL_USE_DMA=1), -DSBL_USE_DMA=1)
  COMP_LIST_COMMON += spi_dma
else
  COMP_LIST_COMMON += spi
endif # ifeq ($(filter $(SBL_CFLAGS), -DSBL_USE_DMA=1), -DSBL_USE_DMA=1)
endif # ifeq ($(filter $(SBL_CFLAGS), -DBOOT_OSPI), -DBOOT_OSPI)

SBL_CFLAGS += -DMAX_APP_SIZE_EMMC=$(MAX_APP_SIZE_EMMC)
SBL_CFLAGS += -DEEPROM_DATA_DDR_ADDRESS=$(EEPROM_DATA_DDR_ADDRESS)

SRCS_COMMON += sbl_main.c

ifeq ($(SBL_ENABLE_BIST), yes)

  SBL_CFLAGS += -DSBL_ENABLE_BIST

  # SDL Include File Paths
  SDL_INSTALL_PATH=$(PDK_INSTALL_PATH)/../../sdl
  INCDIR += $(SDL_INSTALL_PATH)/
  INCDIR += $(SDL_INSTALL_PATH)/src/sdl
  INCDIR += $(SDL_INSTALL_PATH)/include
  INCDIR += $(SDL_INSTALL_PATH)/include/soc/$(SOC_DIR)
  INCDIR += $(SDL_INSTALL_PATH)/osal/
  INCDIR += $(SDL_INSTALL_PATH)/bist/pbist/
  INCDIR += $(SDL_INSTALL_PATH)/bist/lbist/
  INCDIR += $(SDL_INSTALL_PATH)/bist/soc/$(SOC_DIR)/
  INCDIR += $(SDL_INSTALL_PATH)/src/ip/r5

  # PDK Include File Paths
  INCDIR += $(PDK_SBL_COMP_PATH)/example/boot_app/bist
  INCDIR += $(PDK_SBL_COMP_PATH)/example/boot_app/soc/$(SOC_DIR)/bist

  # RM_PM Include File Paths
ifeq ($(SOC_DIR), j721s2)
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/soc/V4/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/pm/soc/j721s2/include/soc/j721s2/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/include/soc/j721s2/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/pm/soc/j721s2/include/
else ifeq ($(SOC_DIR), j784s4)
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/soc/V6/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/pm/soc/j784s4/include/soc/j784s4/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/include/soc/j784s4/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/pm/soc/j784s4/include/
endif
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/pm/include/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/rm_pm_hal/include/
  INCDIR += $(PDK_INSTALL_PATH)/ti/drv/sciclient/src/

  # SDL Source File Paths
  SRCDIR += $(SDL_INSTALL_PATH)/bist/pbist/
  SRCDIR += $(SDL_INSTALL_PATH)/bist/lbist/
  SRCDIR += $(SDL_INSTALL_PATH)/bist/soc/$(SOC_DIR)/

  # PDK Source File Paths
  SRCDIR += $(PDK_SBL_COMP_PATH)/example/boot_app
  SRCDIR += $(PDK_SBL_COMP_PATH)/example/boot_app/bist
  SRCDIR += $(PDK_SBL_COMP_PATH)/example/boot_app/soc/$(SOC_DIR)/bist

  # SDL Integration
  EXT_LIB_LIST_COMMON += $(SDL_INSTALL_PATH)/binary/osal/lib/$(SOC)/r5f/$(BUILD_PROFILE)/sdl_osal.$(LIBEXT)
  EXT_LIB_LIST_COMMON += $(SDL_INSTALL_PATH)/binary/src/ip/lib/$(SOC)/r5f/$(BUILD_PROFILE)/sdl_ip.$(LIBEXT)
  EXT_LIB_LIST_COMMON += $(SDL_INSTALL_PATH)/binary/src/sdl/lib/$(SOC)/r5f/$(BUILD_PROFILE)/sdl_api.$(LIBEXT)
  SRCS_COMMON += boot_app_osal_wrap.c
  SRCS_COMMON += sbl_pbist.c
  SRCS_COMMON += bist.c bist_core_defs.c
  SRCS_COMMON += lbist_utils.c lbist_defs.c
  SRCS_COMMON += pbist_utils.c pbist_defs.c
  SRCS_COMMON += power_seq.c armv8_power_utils.c       

endif

EXTERNAL_LNKCMD_FILE_LOCAL = $(PDK_SBL_COMP_PATH)/soc/k3/$(SOC_DIR)/linker.cmd

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

# EFUSE_DEFAULT - R5 ROM will run the SBL in lockstep mode in lockstep
# #                 enabled devices and in split mode, if the devices do
# #                 not support lockstep.
# # SPLIT_MODE -    R5 ROM will awlys run the SBL on MCU1_0 in split mode
# #                 irrespective of whether the EFuse says the device can
# #                 support lockstep mode.
R5_STARTUP_MODE := SPLIT_MODE
export R5_STARTUP_MODE

include $(MAKERULEDIR)/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk
#     and need not be explicitly specified here

# Nothing beyond this point
