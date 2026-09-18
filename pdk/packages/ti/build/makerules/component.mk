#
# Copyright (c) 2013-2024, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# *  Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# *  Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# *  Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# File: component.mk
#       This file is component include make file of PDK.
# List of variables set in this file and their purpose:
# <mod>_RELPATH        - This is the relative path of the module, typically from
#                        top-level directory of the package
# <mod>_PATH           - This is the absolute path of the module. It derives from
#                        absolute path of the top-level directory (set in env.mk)
#                        and relative path set above
# <mod>_INCLUDE        - This is the path that has interface header files of the
#                        module. This can be multiple directories (space separated)
# <mod>_PKG_LIST       - Names of the modules (and sub-modules) that are a part
#                        part of this module, including itself.
# <mod>_BOARD_DEPENDENCY    - "yes": means the code for this module depends on
#                             board and the compiled obj/lib has to be kept
#                             under <board> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no board dependent code and hence
#                             the obj/libs are not kept under <board> dir.
# <mod>_CORE_DEPENDENCY     - "yes": means the code for this module depends on
#                             core and the compiled obj/lib has to be kept
#                             under <core> directory
#                             "no" or "" or if this variable is not defined: means
#                             this module has no core dependent code and hence
#                             the obj/libs are not kept under <core> dir.
# <mod>_APP_STAGE_FILES     - List of source files that belongs to the module
#                             <mod>, but that needs to be compiled at application
#                             build stage (in the context of the app). This is
#                             primarily for link time configurations or if the
#                             source file is dependent on options/defines that are
#                             application dependent. This can be left blank or
#                             not defined at all, in which case, it means there
#                             no source files in the module <mod> that are required
#                             to be compiled in the application build stage.
# <mod>_FP_DEPENDENCY       - "yes": means the code for this module depends on
#                             target toolchain floating point support.  Enabling
#                             this option will enable floating point print
#                             support for toolchains which link out floating
#                             point print support to save memory.
#
ifeq ($(pdk_component_make_include), )

include $(PDK_INSTALL_PATH)/ti/build/soc_info.mk

# The below defines the DEFAULT_$(SOC)_CORELIST which are used by various components for an SOC
# This is derived from the CORELIST_$(SOC) defined in soc_info.mk which encompasses all the available cores for a SOC.
# The DEFAULT_$(SOC)_CORELIST is a subset of all the cores and is used for building components.

DEFAULT_CORELIST_EXCLUDE_CORES =

# For J7 SOCs, any mpu core other than mpu1_0 is not a part of default core list
ifeq ($(SOC),$(filter $(SOC), j721e j7200 j721s2))
DEFAULT_CORELIST_EXCLUDE_CORES += mpu1_1
endif
ifeq ($(SOC),$(filter $(SOC), j784s4))
DEFAULT_CORELIST_EXCLUDE_CORES += mpu1_1 mpu1_2 mpu1_3 mpu2_0 mpu2_1 mpu2_2 mpu2_3
endif
ifeq ($(SOC),$(filter $(SOC), j742s2))
DEFAULT_CORELIST_EXCLUDE_CORES += mpu1_1 mpu1_2 mpu1_3
endif

DEFAULT_$(SOC)_CORELIST = $(filter-out $(DEFAULT_CORELIST_EXCLUDE_CORES), $(CORE_LIST_$(SOC)))

# The below defines various RTOS types
DEFAULT_RTOS_LIST = freertos

# The below defines the DEFAULT_SOCLIST_<rtos_type> for various RTOS types(freertos/safertos)
DEFAULT_SOCLIST_tirtos   =
DEFAULT_SOCLIST_freertos = j721e j7200 j721s2 j784s4 j742s2
DEFAULT_SOCLIST_safertos = j721e j7200 j721s2 j784s4 j742s2

# The below defines the DEFAULT_BOARDLIST_<rtos_type> for various RTOS types(freertos/safertos)
DEFAULT_BOARDLIST_tirtos   =
DEFAULT_BOARDLIST_freertos = j721e_evm j7200_evm j721s2_evm j784s4_evm j742s2_evm
DEFAULT_BOARDLIST_safertos = j721e_evm j7200_evm j721s2_evm j784s4_evm j742s2_evm

# The below defines the DEFAULT_$(SOC)_CORELIST_<rtos_type> for various RTOS types(tirtos/freertos/safertos)
# This is derived from the DEFAULT_$(SOC)_CORELIST defined above.
# DEFAULT_$(SOC)_CORELIST_<rtos_type> is a subset of all the cores and is used for building components for the particular 'rtos_type'.


ifeq ($(SOC),$(filter $(SOC), j721e j7200 j721s2 j784s4 j742s2))
# FreeRTOS is not supported on mpu core
DEFAULT_CORELIST_EXCLUDE_CORES_freertos += mpu1_0
else
# FreeRTOS is not supported on other SOCs
DEFAULT_CORELIST_EXCLUDE_CORES_freertos = $(DEFAULT_$(SOC)_CORELIST)
endif

DEFAULT_$(SOC)_CORELIST_freertos = $(filter-out $(DEFAULT_CORELIST_EXCLUDE_CORES_freertos), $(DEFAULT_$(SOC)_CORELIST))

ifeq ($(SOC),$(filter $(SOC), j721e j7200 j721s2 j784s4 j742s2))
# SafeRTOS is not currently supported on J7 MPU/C7x-hostemu cores
DEFAULT_CORELIST_EXCLUDE_CORES_safertos = mpu1_0 c7x-hostemu
else
#SafeRTOS is not supported on other SOCs
DEFAULT_CORELIST_EXCLUDE_CORES_safertos = $(DEFAULT_$(SOC)_CORELIST)
endif

DEFAULT_$(SOC)_CORELIST_safertos = $(filter-out $(DEFAULT_CORELIST_EXCLUDE_CORES_safertos), $(DEFAULT_$(SOC)_CORELIST))


# Core types (without the core IDs). This will be used to parse and order the establish the order of cores
# in the case of building libraries for multiple cores

# Component specific CFLAGS
PDK_CFLAGS =
PDK_LNKFLAGS =

############################
# device pdk package
# List of components included under pdk lib
# The components included here are built and will be part of pdk lib
############################
pdk_LIB_LIST =

############################
# device pdk firmware
# List of components included under pdk firmware
# The components included here are built and will be part of pdk firmware
############################
pdk_FIRM_LIST =

############################
# device pdk application utility packages
# List of application utilities under pdk
# The components included here are built and will be part of pdk app_lib
############################
pdk_APP_LIB_LIST =

############################
# device pdk examples
# List of examples under pdk
# All the tests mentioned in list are built when test target is called
# List below all examples for allowed values
############################
pdk_EXAMPLE_LIST =

############################
# Duplicate example list which should not be built with "all" (since this is
# duplicate -j option will build both and result in .obj correuption) but needs
# to be still exposed to top level makefile so that user can individually call
# them. Also these duplicate targets doesn't support package option and hence
# should not be included when packaging
############################
pdk_DUP_EXAMPLE_LIST =

###########################
# PDK Utils list which are not built on every build
# Example includes the sciclient board configruation.
# This is specifically used when some additional tools are required to build
# these utilities like python.
###########################
pdk_UTILS_LIST =

# Components included independent of OS type

#include each module component makefile
-include $(PDK_SDL_COMP_PATH)/sdl_component.mk
ifneq ($(sdl_LIB_LIST),)
  pdk_LIB_LIST += $(sdl_LIB_LIST)
endif

-include $(PDK_CSL_COMP_PATH)/csl_component.mk
ifneq ($(csl_LIB_LIST),)
  pdk_LIB_LIST += $(csl_LIB_LIST)
endif
ifneq ($(csl_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(csl_APP_LIB_LIST)
endif
ifneq ($(csl_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(csl_EXAMPLE_LIST)
endif
ifneq ($(csl_DUP_EXAMPLE_LIST),)
  pdk_DUP_EXAMPLE_LIST += $(csl_DUP_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_BOARD_COMP_PATH)/board_component.mk
ifneq ($(board_LIB_LIST),)
  pdk_LIB_LIST += $(board_LIB_LIST)
endif
ifneq ($(board_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(board_APP_LIB_LIST)
endif
ifneq ($(board_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(board_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_PM_COMP_PATH)/pm_component.mk
ifneq ($(pm_LIB_LIST),)
  pdk_LIB_LIST += $(pm_LIB_LIST)
endif
ifneq ($(pm_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(pm_APP_LIB_LIST)
endif
ifneq ($(pm_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(pm_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_PMIC_COMP_PATH)/pmic_component.mk
ifneq ($(pmic_LIB_LIST),)
  pdk_LIB_LIST += $(pmic_LIB_LIST)
endif
ifneq ($(pmic_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(pmic_APP_LIB_LIST)
endif
ifneq ($(pmic_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(pmic_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_SBL_COMP_PATH)/sbl_component.mk
ifneq ($(sbl_LIB_LIST),)
  pdk_LIB_LIST += $(sbl_LIB_LIST)
endif
ifneq ($(sbl_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(sbl_APP_LIB_LIST)
endif
ifneq ($(sbl_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(sbl_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_KEYWR_COMP_PATH)/keywriter_component.mk
ifneq ($(keywriter_LIB_LIST),)
  pdk_LIB_LIST += $(keywriter_LIB_LIST)
endif
ifneq ($(keywriter_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(keywriter_APP_LIB_LIST)
endif
ifneq ($(keywriter_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(keywriter_EXAMPLE_LIST)
endif
ifneq ($(keywriter_UTILS_LIST),)
  pdk_UTILS_LIST += $(keywriter_UTILS_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_BOARD_IND_COMP_PATH)/board_ind_component.mk
ifneq ($(board_ind_LIB_LIST),)
  pdk_LIB_LIST += $(board_ind_LIB_LIST)
endif
ifneq ($(board_ind_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(board_ind_APP_LIB_LIST)
endif
ifneq ($(board_ind_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(board_ind_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_UDMA_COMP_PATH)/udma_component.mk
ifneq ($(udma_LIB_LIST),)
  pdk_LIB_LIST += $(udma_LIB_LIST)
endif
ifneq ($(udma_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(udma_APP_LIB_LIST)
endif
ifneq ($(udma_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(udma_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_LPM_COMP_PATH)/lpm_component.mk
ifneq ($(lpm_LIB_LIST),)
  pdk_LIB_LIST += $(lpm_LIB_LIST)
endif
ifneq ($(lpm_stub_LIB_LIST),)
  pdk_LIB_LIST += $(lpm_stub_LIB_LIST)
endif
ifneq ($(lpm_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(lpm_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_ENET_COMP_PATH)/enet_component.mk
ifneq ($(enet_LIB_LIST),)
  pdk_LIB_LIST += $(enet_LIB_LIST)
endif
ifneq ($(enet_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(enet_APP_LIB_LIST)
endif
ifneq ($(enet_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(enet_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_LWIP_COMP_PATH)/lwip_component.mk
ifneq ($(lwip_LIB_LIST),)
  pdk_LIB_LIST += $(lwip_LIB_LIST)
endif
ifneq ($(lwip_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(lwip_APP_LIB_LIST)
endif
ifneq ($(lwip_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(lwip_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include ${PDK_TSN_COMP_PATH}/tsn_component.mk
ifneq ($(tsn_LIB_LIST),)
  pdk_LIB_LIST += $(tsn_LIB_LIST)
endif
ifneq ($(tsn_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(tsn_APP_LIB_LIST)
endif
ifneq ($(tsn_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(tsn_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_CSIRX_COMP_PATH)/csirx_component.mk
ifneq ($(csirx_LIB_LIST),)
  pdk_LIB_LIST += $(csirx_LIB_LIST)
endif
ifneq ($(csirx_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(csirx_APP_LIB_LIST)
endif
ifneq ($(csirx_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(csirx_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_CSITX_COMP_PATH)/csitx_component.mk
ifneq ($(csitx_LIB_LIST),)
  pdk_LIB_LIST += $(csitx_LIB_LIST)
endif
ifneq ($(csitx_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(csitx_APP_LIB_LIST)
endif
ifneq ($(csitx_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(csitx_EXAMPLE_LIST)
endif

# fvid2_component.mk should be included before any video components
# - used to ignore include if component not present
-include $(PDK_FVID2_COMP_PATH)/fvid2_component.mk
ifneq ($(fvid2_LIB_LIST),)
  pdk_LIB_LIST += $(fvid2_LIB_LIST)
endif
ifneq ($(fvid2_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(fvid2_APP_LIB_LIST)
endif
ifneq ($(fvid2_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(fvid2_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_DSS_COMP_PATH)/dss_component.mk
ifneq ($(dss_LIB_LIST),)
  pdk_LIB_LIST += $(dss_LIB_LIST)
endif
ifneq ($(dss_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(dss_APP_LIB_LIST)
endif
ifneq ($(dss_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(dss_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_SCICLIENT_COMP_PATH)/sciclient_component.mk
ifneq ($(sciclient_LIB_LIST),)
  pdk_LIB_LIST += $(sciclient_LIB_LIST)
endif
ifneq ($(sciclient_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(sciclient_APP_LIB_LIST)
endif
ifneq ($(sciclient_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(sciclient_EXAMPLE_LIST)
endif
ifneq ($(sciclient_UTILS_LIST),)
  pdk_UTILS_LIST += $(sciclient_UTILS_LIST)
endif

#Below applicable only for TDA devices

# - used to ignore include if component not present
-include $(PDK_DIAG_COMP_PATH)/diag_component.mk
ifneq ($(diag_LIB_LIST),)
  pdk_LIB_LIST += $(diag_LIB_LIST)
endif
ifneq ($(diag_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(diag_APP_LIB_LIST)
endif
ifneq ($(diag_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(diag_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_DIAG_EX_COMP_PATH)/diag_ex_component.mk
ifneq ($(diag_ex_LIB_LIST),)
  pdk_LIB_LIST += $(diag_ex_LIB_LIST)
endif
ifneq ($(diag_ex_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(diag_ex_APP_LIB_LIST)
endif
ifneq ($(diag_ex_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(diag_ex_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_IPC_COMP_PATH)/ipc_component.mk
ifneq ($(ipc_LIB_LIST),)
  pdk_LIB_LIST += $(ipc_LIB_LIST)
endif
ifneq ($(ipc_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(ipc_APP_LIB_LIST)
endif
ifneq ($(ipc_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(ipc_EXAMPLE_LIST)
endif
ifneq ($(ipc_DUP_EXAMPLE_LIST),)
  pdk_DUP_EXAMPLE_LIST += $(ipc_DUP_EXAMPLE_LIST)
endif

#Below applicable only for TDA devices for backward compatibility;not applicable for PRSDK

# - used to ignore include if component not present
-include $(PDK_NORFLASH_COMP_PATH)/norflash_component.mk
ifneq ($(norflash_LIB_LIST),)
  pdk_LIB_LIST += $(norflash_LIB_LIST)
endif
ifneq ($(norflash_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(norflash_APP_LIB_LIST)
endif
ifneq ($(norflash_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(norflash_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_QSPIFLASH_COMP_PATH)/qspiflash_component.mk
ifneq ($(qspiflash_LIB_LIST),)
  pdk_LIB_LIST += $(qspiflash_LIB_LIST)
endif
ifneq ($(qspiflash_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(qspiflash_APP_LIB_LIST)
endif
ifneq ($(qspiflash_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(qspiflash_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_STW_LLD_COMP_PATH)/stw_lld_component.mk
ifneq ($(stw_lld_LIB_LIST),)
  pdk_LIB_LIST += $(stw_lld_LIB_LIST)
endif
ifneq ($(stw_lld_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(stw_lld_APP_LIB_LIST)
endif
ifneq ($(stw_lld_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(stw_lld_EXAMPLE_LIST)
endif
ifneq ($(stw_lld_DUP_EXAMPLE_LIST),)
  pdk_DUP_EXAMPLE_LIST += $(stw_lld_DUP_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_OSAL_COMP_PATH)/osal_component.mk
ifneq ($(osal_LIB_LIST),)
  pdk_LIB_LIST += $(osal_LIB_LIST)
endif
ifneq ($(osal_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(osal_APP_LIB_LIST)
endif
ifneq ($(osal_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(osal_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_I2C_COMP_PATH)/i2c_component.mk
ifneq ($(i2c_LIB_LIST),)
  pdk_LIB_LIST += $(i2c_LIB_LIST)
endif
ifneq ($(i2c_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(i2c_APP_LIB_LIST)
endif
ifneq ($(i2c_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(i2c_EXAMPLE_LIST)
endif

ifneq ($(i2c_FIRM_LIST),)
  pdk_FIRM_LIST += $(i2c_FIRM_LIST)
endif
# - used to ignore include if component not present
-include $(PDK_UART_COMP_PATH)/uart_component.mk
ifneq ($(uart_LIB_LIST),)
  pdk_LIB_LIST += $(uart_LIB_LIST)
endif
ifneq ($(uart_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(uart_APP_LIB_LIST)
endif
ifneq ($(uart_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(uart_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_SERDES_DIAG_COMP_PATH)/serdes_diag_component.mk
ifneq ($(serdes_diag_LIB_LIST),)
  pdk_LIB_LIST += $(serdes_diag_LIB_LIST)
endif
ifneq ($(serdes_diag_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(serdes_diag_APP_LIB_LIST)
endif
ifneq ($(serdes_diag_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(serdes_diag_EXAMPLE_LIST)
endif

  # - used to ignore include if component not present
-include $(PDK_GPIO_COMP_PATH)/gpio_component.mk
ifneq ($(gpio_LIB_LIST),)
  pdk_LIB_LIST += $(gpio_LIB_LIST)
endif
ifneq ($(gpio_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(gpio_APP_LIB_LIST)
endif
ifneq ($(gpio_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(gpio_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_GTC_COMP_PATH)/gtc_component.mk
ifneq ($(gtc_LIB_LIST),)
  pdk_LIB_LIST += $(gtc_LIB_LIST)
endif
ifneq ($(gtc_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(gtc_APP_LIB_LIST)
endif
ifneq ($(gtc_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(gtc_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_PROFILING_COMP_PATH)/profiling_component.mk
ifneq ($(profiling_LIB_LIST),)
  pdk_LIB_LIST += $(profiling_LIB_LIST)
endif
ifneq ($(profiling_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(profiling_APP_LIB_LIST)
endif
ifneq ($(profiling_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(profiling_EXAMPLE_LIST)
endif
# - used to ignore include if component not present
-include $(PDK_COPYVECS_COMP_PATH)/copyvecs_component.mk
ifneq ($(copyvecs_LIB_LIST),)
  pdk_LIB_LIST += $(copyvecs_LIB_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_FATFS_COMP_PATH)/fatfs_component.mk
ifneq ($(fatfs_LIB_LIST),)
  pdk_LIB_LIST += $(fatfs_LIB_LIST)
endif
ifneq ($(fatfs_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(fatfs_APP_LIB_LIST)
endif
ifneq ($(fatfs_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(fatfs_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_MMCSD_COMP_PATH)/mmcsd_component.mk
ifneq ($(mmcsd_LIB_LIST),)
  pdk_LIB_LIST += $(mmcsd_LIB_LIST)
endif
ifneq ($(mmcsd_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(mmcsd_APP_LIB_LIST)
endif
ifneq ($(mmcsd_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(mmcsd_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_MCASP_COMP_PATH)/mcasp_component.mk
ifneq ($(mcasp_LIB_LIST),)
  pdk_LIB_LIST += $(mcasp_LIB_LIST)
endif
ifneq ($(mcasp_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(mcasp_APP_LIB_LIST)
endif
ifneq ($(mcasp_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(mcasp_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_ICSS_EMAC_COMP_PATH)/icss_emac_component.mk
ifneq ($(icss_emac_LIB_LIST),)
  pdk_LIB_LIST += $(icss_emac_LIB_LIST)
endif
ifneq ($(icss_emac_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(icss_emac_APP_LIB_LIST)
endif
ifneq ($(icss_emac_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(icss_emac_EXAMPLE_LIST)
endif
ifneq ($(icss_emac_FIRM_LIST),)
  pdk_FIRM_LIST += $(icss_emac_FIRM_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_EMAC_COMP_PATH)/emac_component.mk
ifneq ($(emac_LIB_LIST),)
  pdk_LIB_LIST += $(emac_LIB_LIST)
endif
ifneq ($(emac_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(emac_APP_LIB_LIST)
endif
ifneq ($(emac_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(emac_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_PRUSS_COMP_PATH)/pruss_component.mk
ifneq ($(pruss_LIB_LIST),)
  pdk_LIB_LIST += $(pruss_LIB_LIST)
endif
ifneq ($(pruss_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(pruss_APP_LIB_LIST)
endif
ifneq ($(pruss_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(pruss_EXAMPLE_LIST)
endif
ifneq ($(pruss_FIRM_LIST),)
  pdk_FIRM_LIST += $(pruss_FIRM_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_SPI_COMP_PATH)/spi_component.mk
ifneq ($(spi_LIB_LIST),)
  pdk_LIB_LIST += $(spi_LIB_LIST)
endif
ifneq ($(spi_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(spi_APP_LIB_LIST)
endif
ifneq ($(spi_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(spi_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_SA_COMP_PATH)/sa_component.mk
ifneq ($(sa_LIB_LIST),)
  pdk_LIB_LIST += $(sa_LIB_LIST)
endif
ifneq ($(sa_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(sa_APP_LIB_LIST)
endif
ifneq ($(sa_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(sa_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_MAILBOX_COMP_PATH)/mailbox_component.mk
ifneq ($(mailbox_LIB_LIST),)
  pdk_LIB_LIST += $(mailbox_LIB_LIST)
endif
ifneq ($(mailbox_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(mailbox_APP_LIB_LIST)
endif
ifneq ($(mailbox_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(mailbox_EXAMPLE_LIST)
endif
ifneq ($(mailbox_DUP_EXAMPLE_LIST),)
  pdk_DUP_EXAMPLE_LIST += $(mailbox_DUP_EXAMPLE_LIST)
endif

-include $(PDK_CANFD_COMP_PATH)/canfd_component.mk
ifneq ($(canfd_LIB_LIST),)
  pdk_LIB_LIST += $(canfd_LIB_LIST)
endif
ifneq ($(canfd_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(canfd_APP_LIB_LIST)
endif
ifneq ($(canfd_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(canfd_EXAMPLE_LIST)
endif

-include $(PDK_FREERTOS_COMP_PATH)/freertos_component.mk
ifneq ($(freertos_LIB_LIST),)
  pdk_LIB_LIST += $(freertos_LIB_LIST)
endif
ifneq ($(freertos_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(freertos_APP_LIB_LIST)
endif
ifneq ($(freertos_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(freertos_EXAMPLE_LIST)
endif

-include $(PDK_SAFERTOS_COMP_PATH)/safertos_component.mk
ifneq ($(safertos_LIB_LIST),)
  pdk_LIB_LIST += $(safertos_LIB_LIST)
endif
ifneq ($(safertos_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(safertos_APP_LIB_LIST)
endif
ifneq ($(safertos_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(safertos_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_CAN_COMP_PATH)/can_component.mk
ifneq ($(can_LIB_LIST),)
  pdk_LIB_LIST += $(can_LIB_LIST)
endif
ifneq ($(can_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $(can_APP_LIB_LIST)
endif
ifneq ($(can_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(can_EXAMPLE_LIST)
endif

  #Below applicable only for K3 DMSC; not applicable for PRSDK
  # - used to ignore include if component not present
  -include $(DMSC_COMP_PATH)/dmsc_component.mk
  ifneq ($(dmsc_LIB_LIST),)
    pdk_LIB_LIST += $(dmsc_LIB_LIST)
  endif
  ifneq ($(dmsc_EXAMPLE_LIST),)
    pdk_EXAMPLE_LIST += $(dmsc_EXAMPLE_LIST)
  endif

# - used to ignore include if component not present
-include $(PDK_BOARD_UTILS_COMP_PATH)/board_utils_component.mk
ifneq ($(board_utils_LIB_LIST),)
  pdk_LIB_LIST += $(board_utils_LIB_LIST)
endif
ifneq ($(board_utils_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(board_utils_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDK_BOARD_DIAG_COMP_PATH)/board_diag_component.mk
ifneq ($(board_diag_LIB_LIST),)
  pdk_LIB_LIST += $(board_diag_LIB_LIST)
endif
ifneq ($(board_diag_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(board_diag_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(SAFETY_CHECKERS_COMP_PATH)/safety_checkers_component.mk
ifneq ($(safety_checkers_LIB_LIST),)
  pdk_LIB_LIST += $(safety_checkers_LIB_LIST)
endif
ifneq ($(safety_checkers_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(safety_checkers_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(IP_FMA_COMP_PATH)/ip_fma_component.mk
ifneq ($(ip_fma_LIB_LIST),)
  pdk_LIB_LIST += $(ip_fma_LIB_LIST)
endif
ifneq ($(ip_fma_EXAMPLE_LIST),)
 pdk_EXAMPLE_LIST += $(ip_fma_EXAMPLE_LIST)
endif

# - used to ignore include if component not present
-include $(PDM_UTILS_COMP_PATH)/pdm_utils_component.mk
ifneq ($(pdm_utils_LIB_LIST),)
  pdk_LIB_LIST += $(pdm_utils_LIB_LIST)
endif
ifneq ($(pdm_utils_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $(pdm_utils_EXAMPLE_LIST)
endif

# Adding new component to the packages/ti/build which is not part of the PDK.
# An example would be some demo which is not built from packages/ti/build but
# would still like to use the ti/build/ infrastructure

ifeq ($($(COMP)_USE_PDK_BUILD),yes)
include $($(COMP)_COMPONENT_MK_PATH)/$(COMP)_component.mk

ifneq ($($(COMP)_LIB_LIST),)
  pdk_LIB_LIST += $($(COMP)_LIB_LIST)
endif

ifneq ($($(COMP)_APP_LIB_LIST),)
  pdk_APP_LIB_LIST += $($(COMP)_APP_LIB_LIST)
endif

ifneq ($($(COMP)_FIRM_LIST),)
  pdk_FIRM_LIST += $($(COMP)_FIRM_LIST)
endif

ifneq ( $($(COMP)_EXAMPLE_LIST),)
  pdk_EXAMPLE_LIST += $($(COMP)_EXAMPLE_LIST)
endif

endif

pdk_PKG_LIST_ALL = $(pdk_EXAMPLE_LIST) $(pdk_LIB_LIST) $(pdk_APP_LIB_LIST) $(pdk_FIRM_LIST)

LIB_ENDIAN_LIST = little
#
# PDK Modules
#

#
# PDK Examples
#

#Default to all if not set from rules.make or command line
ifeq ($(PACKAGE_SELECT), )
  PACKAGE_SELECT := all
endif

ifeq ($(CORE),mcu1_0)
  PDK_CFLAGS += -DBUILD_MCU1_0 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU1_0 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu1_1)
  PDK_CFLAGS += -DBUILD_MCU1_1 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU1_1 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu2_0)
  PDK_CFLAGS += -DBUILD_MCU2_0 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU2_0 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu2_1)
  PDK_CFLAGS += -DBUILD_MCU2_1 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU2_1 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu3_0)
  PDK_CFLAGS += -DBUILD_MCU3_0 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU3_0 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu3_1)
  PDK_CFLAGS += -DBUILD_MCU3_1 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU3_1 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu4_0)
  PDK_CFLAGS += -DBUILD_MCU4_0 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU4_0 --define=BUILD_MCU
endif

ifeq ($(CORE),mcu4_1)
  PDK_CFLAGS += -DBUILD_MCU4_1 -DBUILD_MCU
  PDK_LNKFLAGS += --define=BUILD_MCU4_1 --define=BUILD_MCU
endif

ifeq ($(CORE),mpu1_0)
  PDK_CFLAGS += -DBUILD_MPU1_0 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU1_0 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu1_1)
  PDK_CFLAGS += -DBUILD_MPU1_1 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU1_1 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu1_2)
  PDK_CFLAGS += -DBUILD_MPU1_2 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU1_2 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu1_3)
  PDK_CFLAGS += -DBUILD_MPU1_3 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU1_3 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu2_0)
  PDK_CFLAGS += -DBUILD_MPU2_0 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU2_0 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu2_1)
  PDK_CFLAGS += -DBUILD_MPU2_1 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU2_1 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu2_2)
  PDK_CFLAGS += -DBUILD_MPU2_2 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU2_2 --define=BUILD_MPU
endif

ifeq ($(CORE),mpu2_3)
  PDK_CFLAGS += -DBUILD_MPU2_3 -DBUILD_MPU
  PDK_LNKFLAGS += --define=BUILD_MPU2_3 --define=BUILD_MPU
endif

ifeq ($(CORE),$(filter $(CORE), c66xdsp_1 c66xdsp_2))
  ifeq ($(CORE),$(filter $(CORE), c66xdsp_1))
    PDK_CFLAGS += -DBUILD_DSP_1 -DBUILD_C66X_1  -DBUILD_C66X
    PDK_LNKFLAGS += --define=BUILD_DSP_1 --define=BUILD_C66X_1 --define=BUILD_C66X
  endif
  ifeq ($(CORE),$(filter $(CORE), c66xdsp_2))
    PDK_CFLAGS += -DBUILD_DSP_2 -DBUILD_C66X_2 -DBUILD_C66X
    PDK_LNKFLAGS += --define=BUILD_DSP_2 --define=BUILD_C66X_2 --define=BUILD_C66X
  endif
endif

ifeq ($(CORE),$(filter $(CORE), c7x_1 c7x-hostemu))
  PDK_CFLAGS += -DBUILD_C7X_1 -DBUILD_C7X
  PDK_LNKFLAGS += --define=BUILD_C7X_1 --define=BUILD_C7X
endif

ifeq ($(CORE),$(filter $(CORE), c7x_2))
  PDK_CFLAGS += -DBUILD_C7X_2 -DBUILD_C7X
  PDK_LNKFLAGS += --define=BUILD_C7X_2 --define=BUILD_C7X
endif

ifeq ($(CORE),$(filter $(CORE), c7x_3))
  PDK_CFLAGS += -DBUILD_C7X_3 -DBUILD_C7X
  PDK_LNKFLAGS += --define=BUILD_C7X_3 --define=BUILD_C7X
endif

ifeq ($(CORE),$(filter $(CORE), c7x_4))
  PDK_CFLAGS += -DBUILD_C7X_4 -DBUILD_C7X
  PDK_LNKFLAGS += --define=BUILD_C7X_4 --define=BUILD_C7X
endif

ifeq ($(CORE),$(filter $(CORE), m4f_0))
PDK_CFLAGS += -DBUILD_M4F_0 -DBUILD_M4F
 PDK_LNKFLAGS += --define=BUILD_M4F_0 --define=BUILD_M4F
endif

export PDK_CFLAGS
export PDK_LNKFLAGS

ifeq ($(SOC),$(filter $(SOC), j721e))
  PDK_COMMON_COMP = csl uart i2c udma gpio pmic pm_lib gtc
  ifneq ($(CORE),$(filter $(CORE), c7x-hostemu))
    PDK_COMMON_COMP += board
  endif
  ifeq ($(CORE),mcu1_0)
    PDK_COMMON_COMP += sciclient_direct rm_pm_hal
  else
    PDK_COMMON_COMP += sciclient
  endif
endif

ifeq ($(SOC),$(filter $(SOC), j7200))
  PDK_COMMON_COMP = csl uart i2c board udma gpio pmic pm_lib gtc
  ifeq ($(CORE),mcu1_0)
    PDK_COMMON_COMP += rm_pm_hal lpm_stub sciclient_direct
  else
    PDK_COMMON_COMP += sciclient
  endif
endif

ifeq ($(SOC),$(filter $(SOC), j721s2 j784s4 j742s2))
  PDK_COMMON_COMP = csl uart board udma i2c gpio pm_lib gtc
  ifeq ($(CORE),mcu1_0)
    PDK_COMMON_COMP += rm_pm_hal lpm_stub sciclient_direct
  else
    PDK_COMMON_COMP += sciclient
  endif
endif

PDK_COMMON_BAREMETAL_COMP = $(PDK_COMMON_COMP) osal_nonos
ifeq ($(ARCH),c66x)
  PDK_COMMON_BAREMETAL_COMP += csl_intc
else
  ifneq ($(ARCH),c71)
    PDK_COMMON_BAREMETAL_COMP += csl_init
  endif
endif

ifeq ($(SOC),$(filter $(SOC), j721e j7200 j721s2 j784s4 j742s2))
  PDK_COMMON_FREERTOS_COMP = $(PDK_COMMON_COMP) osal_freertos
  PDK_COMMON_FREERTOS_COMP += freertos
  PDK_COMMON_SAFERTOS_COMP = $(PDK_COMMON_COMP) osal_safertos
  PDK_COMMON_SAFERTOS_COMP += safertos
ifeq ($(ARCH),c66x)
  PDK_COMMON_FREERTOS_COMP += csl_intc
  PDK_COMMON_SAFERTOS_COMP += csl_intc
else
  PDK_COMMON_FREERTOS_COMP += csl_init
  PDK_COMMON_SAFERTOS_COMP += csl_init
endif
endif

export PDK_COMMON_BAREMETAL_COMP
export PDK_COMMON_FREERTOS_COMP
export PDK_COMMON_SAFERTOS_COMP

pdk_component_make_include := 1
endif
