#
# Copyright (c) 2025, Texas Instruments Incorporated
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

# File: lpm_component.mk
#       This file is component include make file of lpm library.
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
# <mod>_BOARD_DEPENDENCY - "yes": means the code for this module depends on
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
#
ifeq ($(lpm_component_make_include), )

lpm_lib_BOARDLIST       = j721e_evm j7200_evm j784s4_evm j742s2_evm j721s2_evm
lpm_lib_j721e_CORELIST  = mcu1_0
lpm_lib_j7200_CORELIST  = mcu1_0
lpm_lib_j784s4_CORELIST  = mcu1_0

lpm_RTOS_LIST = $(DEFAULT_RTOS_LIST)

############################
# lpm package
# List of components included under lpm lib
# The components included here are built and will be part of lpm lib
############################

#
# lpm Modules
#

# lpm LIB
lpm_COMP_LIST = lpm
lpm_RELPATH = ti/drv/lpm
lpm_PATH = $(PDK_LPM_COMP_PATH)
export lpm_LIBNAME = lpm
export lpm_LIBPATH = $(lpm_PATH)/lib
export lpm_MAKEFILE = -fsrc/makefile
export lpm_BOARD_DEPENDENCY = yes
export lpm_CORE_DEPENDENCY = no
export lpm_SOC_DEPENDENCY = yes
lpm_PKG_LIST = lpm
lpm_INCLUDE = $(lpm_PATH)
export lpm_SOCLIST = j721e j7200 j784s4 j742s2 j721s2

# Only SOCs with actual lpm library support get a CORELIST (others use stub)
ifneq ($(filter $(SOC), j721e j7200 j784s4),)
export lpm_$(SOC)_CORELIST = mcu1_0
endif
lpm_LIB_LIST = lpm

# lpm S2R
lpm_stub_COMP_LIST = lpm_stub
lpm_stub_RELPATH = $(lpm_RELPATH)
lpm_stub_PATH = $(PDK_LPM_COMP_PATH)
export lpm_stub_LIBNAME = lpm_stub
export lpm_stub_LIBPATH = $(lpm_PATH)/lib
export lpm_stub_MAKEFILE = -fsrc/lpm_stub/makefile
export lpm_stub_BOARD_DEPENDENCY = yes
export lpm_stub_CORE_DEPENDENCY = no
export lpm_stub_SOC_DEPENDENCY = yes
export lpm_stub_SOCLIST = j7200 j721s2 j784s4 j742s2
export lpm_stub_$(SOC)_CORELIST = mcu1_0
lpm_stub_LIB_LIST = lpm_stub
lpm_stub_PKG_LIST = lpm_stub

############################
# lpm examples
# List of examples under lpm
# All the tests mentioned in list are built when test target is called
# List below all examples for allowed values
############################
lpm_EXAMPLE_LIST =

#
# lpm Examples
#

# lpm RTOS DDR thermal monitor test apps
define LPM_EXAMPLE_RULE

export lpm_example_$(1)_COMP_LIST = lpm_example_$(1)
lpm_example_$(1)_RELPATH = ti/drv/lpm/examples/lpm_example
lpm_example_$(1)_PATH = $(PDK_LPM_COMP_PATH)/examples/lpm_example
export lpm_example_$(1)_BOARD_DEPENDENCY = yes
export lpm_example_$(1)_CORE_DEPENDENCY = yes
export lpm_example_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
lpm_example_$(1)_PKG_LIST = lpm_example_$(1)
lpm_example_$(1)_INCLUDE = $(lpm_example_$(1)_PATH)
export lpm_example_$(1)_BOARDLIST = j721e_evm j7200_evm j784s4_evm
export lpm_example_$(1)_$(SOC)_CORELIST = $(filter $(DEFAULT_$(SOC)_CORELIST_$(1)), $(lpm_$(SOC)_CORELIST))
export lpm_example_$(1)_SBL_APPIMAGEGEN = yes

lpm_EXAMPLE_LIST += lpm_example_$(1)


export lpm_io_retention_$(1)_COMP_LIST = lpm_io_retention_$(1)
lpm_io_retention_$(1)_RELPATH = ti/drv/lpm/examples/lpm_io_retention
lpm_io_retention_$(1)_PATH = $(PDK_LPM_COMP_PATH)/examples/lpm_io_retention
export lpm_io_retention_$(1)_BOARD_DEPENDENCY = yes
export lpm_io_retention_$(1)_CORE_DEPENDENCY = yes
export lpm_io_retention_$(1)_MAKEFILE = -f makefile BUILD_OS_TYPE=$(1)
lpm_io_retention_$(1)_PKG_LIST = lpm_io_retention_$(1)
lpm_io_retention_$(1)_INCLUDE = $(lpm_io_retention_$(1)_PATH)
export lpm_io_retention_$(1)_BOARDLIST = j7200_evm
export lpm_io_retention_$(1)_$(SOC)_CORELIST = mcu1_0
export lpm_io_retention_$(1)_SBL_APPIMAGEGEN = yes

lpm_EXAMPLE_LIST += lpm_io_retention_$(1)

endef

define LPM_BOARDLIST_RULE
lpm_$(1)_BOARDLIST = j721e_evm j7200_evm j784s4_evm
endef

LPM_EXAMPLE_MACRO_LIST := $(foreach curos, $(lpm_RTOS_LIST), $(call LPM_EXAMPLE_RULE,$(curos)))

$(eval ${LPM_EXAMPLE_MACRO_LIST})

ifneq ($(BUILD_PROFILE), release)
lpm_EXAMPLE_LIST =
endif

export lpm_LIB_LIST
export lpm_stub_LIB_LIST
export lpm_EXAMPLE_LIST

lpm_component_make_include := 1
endif

