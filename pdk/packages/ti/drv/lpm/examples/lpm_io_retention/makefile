# Copyright (C) 2019-2024 Texas Instruments Incorporated - http://www.ti.com/
#
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# Macro definitions referenced below
#

#
# This file is the makefile for building Leo PMIC mode transition app.
#
#
# This file is common makefile for building LPM example for both TI-RTOS/baremetal
#
ifeq ($(RULES_MAKE), )
include $(PDK_INSTALL_PATH)/ti/build/Rules.make
else
include $(RULES_MAKE)
endif

APP_NAME = lpm_io_retention_$(BUILD_OS_TYPE)

SRCDIR = src ../../../../board/diag/common/${SOC}/
INCDIR = ../../include ../../include/io_retention ../../../../board/src/${BOARD}/include ../../../../board/diag/common/${SOC}/ 


# List all the external components/interfaces, whose interface header files
#  need to be included for this component
INCLUDE_EXTERNAL_INTERFACES = pdk

COMP_LIST_COMMON = $(PDK_COMMON_FREERTOS_COMP) lpm sciserver_tirtos ipc sbl_lib_cust spi
INCLUDE_EXTERNAL_INTERFACES += freertos board
SRCS_COMMON = main_rtos.c overrides/$(SOC)/$(CORE)/r5_mpu_freertos.c ../../../gpio/test/led_blink/src/GPIO_board.c
EXTERNAL_LNKCMD_FILE_LOCAL = overrides/$(SOC)/$(CORE)/linker_r5_freertos.lds

# Common source files and CFLAGS across all platforms and cores
PACKAGE_SRCS_COMMON = .
SRCS_COMMON += ../../../lpm/src/lpm_uart_drv.c

CFLAGS_LOCAL_COMMON = $(PDK_CFLAGS) -DDEVICE_NAME=j7vcl_core 

# Uncomment below line to print debug logs.
# CFLAGS_LOCAL_COMMON += -DLPM_DEBUG_ENABLE

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
