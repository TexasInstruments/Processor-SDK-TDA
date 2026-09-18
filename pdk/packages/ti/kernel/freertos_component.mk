#
# Copyright (c) 2016 - 2020, Texas Instruments Incorporated
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

# File: freertos_component.mk
#       This file is component include make file of freertos library.
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
ifeq ($(freertos_component_make_include), )

freertos_BOARDLIST       = tpr12_evm j721e_evm j7200_evm am65xx_evm am65xx_idk awr294x_evm j721s2_evm j784s4_evm j742s2_evm
freertos_SOCLIST         = tpr12 j721e j7200 am65xx awr294x j721s2 j784s4 j742s2
freertos_tpr12_CORELIST   = c66xdsp_1 mcu1_0 mcu1_1
freertos_awr294x_CORELIST   = c66xdsp_1 mcu1_0 mcu1_1
freertos_j721e_CORELIST   = mcu1_0 mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1 c66xdsp_1 c66xdsp_2 c7x_1
freertos_j7200_CORELIST   = mcu1_0 mcu1_1 mcu2_0 mcu2_1
freertos_am65xx_CORELIST   = mcu1_0 mcu1_1
freertos_j721s2_CORELIST   = mcu1_0 mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1 c7x_1 c7x_2
freertos_j784s4_CORELIST   = mcu1_0 mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1 mcu4_0 mcu4_1 c7x_1 c7x_2 c7x_3 c7x_4
freertos_j742s2_CORELIST   = mcu1_0 mcu1_1 mcu2_0 mcu2_1 mcu3_0 mcu3_1 mcu4_0 mcu4_1 c7x_1 c7x_2 c7x_3

############################
# freertos package
# List of components included under freertos lib
# The components included here are built and will be part of freertos lib
############################
freertos_LIB_LIST = freertos



############################
# freertos examples
# List of examples under freertos
# All the tests mentioned in list are built when test target is called
# List below all examples for allowed values
############################
freertos_EXAMPLE_LIST = 

ifeq ($(SOC),$(filter $(SOC), $(freertos_SOCLIST)))
freertos_EXAMPLE_LIST =  freertos_test_task_switch
freertos_EXAMPLE_LIST =  freertos_test_task_context_switch
freertos_EXAMPLE_LIST += freertos_test_ut
ifneq ($(wildcard $(PDK_FREERTOS_COMP_PATH)/freertos_posix.mak),)
ifneq ($(wildcard $(PDK_FREERTOS_COMP_PATH)/test/freertos/posix_demo),)
ifneq ($(wildcard $(FREERTOS_LABS_INSTALL_PATH)/FreeRTOS-POSIX),)
freertos_EXAMPLE_LIST += freertos_test_posix
endif
endif
endif
endif

SOC_DIR=$(SOC)
ifeq ($(SOC), j742s2)
  SOC_DIR=j784s4
endif

#
# freertos Modules
#

# freertos LIB
freertos_COMP_LIST = freertos
freertos_RELPATH = ti/kernel
freertos_PATH = $(PDK_FREERTOS_COMP_PATH)
freertos_LIBNAME = ti.kernel.freertos
export freertos_LIBNAME
freertos_LIBPATH = $(freertos_PATH)/lib
export freertos_LIBPATH
freertos_OBJPATH = $(freertos_RELPATH)
export freertos_OBJPATH
freertos_MAKEFILE = -f freertos.mak
export freertos_MAKEFILE
freertos_BOARD_DEPENDENCY = no
freertos_CORE_DEPENDENCY = yes
freertos_SOC_DEPENDENCY = yes
export freertos_COMP_LIST
export freertos_BOARD_DEPENDENCY
export freertos_CORE_DEPENDENCY
export freertos_SOC_DEPENDENCY
freertos_PKG_LIST = freertos
export freertos_PKG_LIST
freertos_INCLUDE = $(freertos_PATH)
freertos_INCLUDE += ${FREERTOS_KERNEL_INSTALL_PATH}/FreeRTOS-Kernel/include
freertos_INCLUDE += $(PDK_FREERTOS_COMP_PATH)/freertos/portable/TI_CGT/$(ISA)
ifeq ($(ISA), c7x)
freertos_INCLUDE += $(PDK_CSL_COMP_PATH)/arch/c7x
endif
freertos_INCLUDE += $(PDK_FREERTOS_COMP_PATH)/freertos/config/$(SOC_DIR)/$(ISA)

export freertos_SOCLIST
export freertos_$(SOC)_CORELIST



#
# freertos Examples
#
# freertos rtos test app

# freertos rtos task switch test app
freertos_test_task_switch_COMP_LIST = freertos_test_task_switch
freertos_test_task_switch_RELPATH = ti/kernel/test/freertos/task_switch
freertos_test_task_switch_PATH = $(PDK_FREERTOS_COMP_PATH)/test/freertos/task_switch
freertos_test_task_switch_BOARD_DEPENDENCY = yes
freertos_test_task_switch_CORE_DEPENDENCY = no
freertos_test_task_switch_XDC_CONFIGURO = no
freertos_test_task_switch_MAKEFILE = -f makefile
export freertos_test_task_switch_COMP_LIST
export freertos_test_task_switch_BOARD_DEPENDENCY
export freertos_test_task_switch_CORE_DEPENDENCY
export freertos_test_task_switch_XDC_CONFIGURO
export freertos_test_task_switch_MAKEFILE
freertos_test_task_switch_PKG_LIST = freertos_test_task_switch
export freertos_test_task_switch_PKG_LIST
freertos_test_task_switch_INCLUDE = $(freertos_test_task_switch_PATH)
freertos_test_task_switch_BOARDLIST = $(freertos_BOARDLIST)
export freertos_test_task_switch_BOARDLIST
freertos_test_task_switch_$(SOC)_CORELIST = $(freertos_$(SOC)_CORELIST)
export freertos_test_task_switch_$(SOC)_CORELIST
export freertos_test_task_switch_SBL_APPIMAGEGEN = yes

# freertos task context switch test app to validate c7x port context save + restore
freertos_test_task_context_switch_COMP_LIST = freertos_test_task_context_switch
freertos_test_task_context_switch_RELPATH = ti/kernel/test/freertos/task_context_switch
freertos_test_task_context_switch_PATH = $(PDK_FREERTOS_COMP_PATH)/test/freertos/task_context_switch
freertos_test_task_context_switch_BOARD_DEPENDENCY = yes
freertos_test_task_context_switch_CORE_DEPENDENCY = yes
freertos_test_task_context_switch_XDC_CONFIGURO = no
freertos_test_task_context_switch_MAKEFILE = -f makefile
export freertos_test_task_context_switch_COMP_LIST
export freertos_test_task_context_switch_BOARD_DEPENDENCY
export freertos_test_task_context_switch_CORE_DEPENDENCY
export freertos_test_task_context_switch_XDC_CONFIGURO
export freertos_test_task_context_switch_MAKEFILE
freertos_test_task_context_switch_PKG_LIST = freertos_test_task_context_switch
export freertos_test_task_context_switch_PKG_LIST
freertos_test_task_context_switch_INCLUDE = $(freertos_test_task_context_switch_PATH)
freertos_test_task_context_switch_BOARDLIST = j721e_evm j721s2_evm j784s4_evm
export freertos_test_task_context_switch_BOARDLIST
freertos_test_task_context_switch_$(SOC)_CORELIST = c7x_1 c7x_2 c7x_3 c7x_4
export freertos_test_task_context_switch_$(SOC)_CORELIST
export freertos_test_task_context_switch_SBL_APPIMAGEGEN = yes

# freertos rtos unit test app
freertos_test_ut_COMP_LIST = freertos_test_ut
freertos_test_ut_RELPATH = ti/kernel/test/freertos/ut
freertos_test_ut_PATH = $(PDK_FREERTOS_COMP_PATH)/test/freertos/ut
freertos_test_ut_BOARD_DEPENDENCY = yes
freertos_test_ut_CORE_DEPENDENCY = no
freertos_test_ut_XDC_CONFIGURO = no
freertos_test_ut_MAKEFILE = -f makefile
export freertos_test_ut_COMP_LIST
export freertos_test_ut_BOARD_DEPENDENCY
export freertos_test_ut_CORE_DEPENDENCY
export freertos_test_ut_XDC_CONFIGURO
export freertos_test_ut_MAKEFILE
freertos_test_ut_PKG_LIST = freertos_test_ut
export freertos_test_ut_PKG_LIST
freertos_test_ut_INCLUDE = $(freertos_test_ut_PATH)
freertos_test_ut_BOARDLIST = $(freertos_BOARDLIST)
export freertos_test_ut_BOARDLIST
freertos_test_ut_$(SOC)_CORELIST = $(freertos_$(SOC)_CORELIST)
export freertos_test_ut_$(SOC)_CORELIST
export freertos_test_ut_SBL_APPIMAGEGEN = yes

# freertos rtos posix test app
freertos_test_posix_COMP_LIST = freertos_test_posix
freertos_test_posix_RELPATH = ti/kernel/test/freertos/posix_demo
freertos_test_posix_PATH = $(PDK_FREERTOS_COMP_PATH)/test/freertos/posix_demo
freertos_test_posix_BOARD_DEPENDENCY = yes
freertos_test_posix_CORE_DEPENDENCY = no
freertos_test_posix_XDC_CONFIGURO = no
freertos_test_posix_MAKEFILE = -f makefile
export freertos_test_posix_COMP_LIST
export freertos_test_posix_BOARD_DEPENDENCY
export freertos_test_posix_CORE_DEPENDENCY
export freertos_test_posix_XDC_CONFIGURO
export freertos_test_posix_MAKEFILE
freertos_test_posix_PKG_LIST = freertos_test_posix
export freertos_test_posix_PKG_LIST
freertos_test_posix_INCLUDE = $(freertos_test_posix_PATH)
freertos_test_posix_BOARDLIST = $(freertos_BOARDLIST)
export freertos_test_posix_BOARDLIST
ifneq ($(SOC),$(filter $(SOC), j721s2 j784s4 j742s2))
freertos_test_posix_$(SOC)_CORELIST = $(freertos_$(SOC)_CORELIST)
else
ifeq ($(SOC),$(filter $(SOC), j784s4))
# Temporarily disable  FreeRTOS POSIX Demo for J784s4 on c7x_1 C7x_2 c7x_3 c7x_4 cores,
# since UNITY related build failures are observed for these cores
freertos_test_posix_$(SOC)_CORELIST = $(filter-out c7x_1 c7x_2 c7x_3 c7x_4, $(freertos_$(SOC)_CORELIST))
else ifeq ($(SOC),$(filter $(SOC), j742s2))
# Temporarily disable  FreeRTOS POSIX Demo for J742S2 on c7x_1 C7x_2 c7x_3 cores,
# since UNITY related build failures are observed for these cores
freertos_test_posix_$(SOC)_CORELIST = $(filter-out c7x_1 c7x_2 c7x_3, $(freertos_$(SOC)_CORELIST))
else
# Temp disable FreeRTOS POSIX Demo for J721S2 C7x cores, 
# Since build failures are seen with silicon_version 7120 on C7x CGT 2.0.0A21260 
freertos_test_posix_$(SOC)_CORELIST = $(filter-out c7x_1 c7x_2, $(freertos_$(SOC)_CORELIST)) 
endif
endif
export freertos_test_posix_$(SOC)_CORELIST


export freertos_LIB_LIST
export freertos_EXAMPLE_LIST

freertos_component_make_include := 1
endif
