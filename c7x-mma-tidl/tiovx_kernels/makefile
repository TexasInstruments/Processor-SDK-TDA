#
# Copyright (c) 2023 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
# *	   No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
# *	   any redistribution and use are licensed by TI for use only with TI Devices.
#
# *	   Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
# *	   any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
# *	   any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
ALGBASE_PATH ?= $(abspath ../)
PSDK_INSTALL_PATH ?= $(abspath ../../)
PSDK_BUILDER_PATH ?= $(PSDK_INSTALL_PATH)/sdk_builder
CONCERTO_ROOT     ?= $(PSDK_BUILDER_PATH)/concerto
ARM_TIDL_PATH   ?= $(ALGBASE_PATH)/arm-tidl

include $(ALGBASE_PATH)/arm-tidl/config.mk

BUILD_IGNORE_LIB_ORDER := yes
GCC_LINUX_ROOT         := /usr

CGT7X_ROOT := $(DSP_TOOLS)
PDK_PATH   := $(PDK_INSTALL_PATH)

BUILD_TARGET    := $(PSDK_BUILDER_PATH)/target.mak

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721E j721e))
BUILD_DEFS += SOC_J721E
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721S2 j721s2))
BUILD_DEFS += SOC_J721S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J784S4 j784s4))
BUILD_DEFS += SOC_J784S4
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
BUILD_DEFS += SOC_TDA54
BUILD_DEFS += TDA54_VDK_HOST_EMULATION
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J742S2 j742s2))
BUILD_DEFS += SOC_J742S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
BUILD_DEFS += SOC_J722S
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
BUILD_DEFS += SOC_AM62A
BUILD_DEFS += SOC_AM62AX
TARGET_SOC_TIOVX = AM62A
endif

ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
BUILD_DEFS += ENABLE_SDK_9_2_COMPATIBILITY
else ifeq ($(ENABLE_SDK_10_0_COMPATIBILITY), 1)
BUILD_DEFS += ENABLE_SDK_10_0_COMPATIBILITY
else ifeq ($(ENABLE_SDK_10_1_COMPATIBILITY), 1)
BUILD_DEFS += ENABLE_SDK_10_1_COMPATIBILITY
else ifeq ($(ENABLE_SDK_11_0_COMPATIBILITY), 1)
BUILD_DEFS += ENABLE_SDK_11_0_COMPATIBILITY
else ifeq ($(ENABLE_SDK_11_1_COMPATIBILITY), 1)
BUILD_DEFS += ENABLE_SDK_11_1_COMPATIBILITY
endif

DIRECTORIES :=
DIRECTORIES += .

TARGET_COMBOS :=

ifeq ($(TARGET_PLATFORM),PC)
    TARGET_COMBOS += PC:LINUX:x86_64:1:$(TARGET_BUILD):GCC_LINUX
else
    TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:$(TARGET_BUILD):CGT7X
endif

include $(CONCERTO_ROOT)/rules.mak

.PHONY: all
all:
