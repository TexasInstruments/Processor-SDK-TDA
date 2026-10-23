#
# Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
# *       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
# *       any redistribution and use are licensed by TI for use only with TI Devices.
#
# *       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
# *       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
# *       any redistribution and use of any object code compiled from the source code
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
#Other Variables

# Inherit common build flags from root repo in SDK
include $(PSDK_BUILDER_PATH)/build_flags.mak

# Supported values: PC, TI_DEVICE (SIMULATORS or EMULATOR )
TARGET_PLATFORM ?= TI_DEVICE

# Supported values: debug and release)
TARGET_BUILD    ?= release
# Supported C64T, C64P, C64, C66, C674, C67, C67P, m4
TARGET_CPU      ?= C71
# Supported values: yes and no)
BUILD_WITH_CUDA ?= no
# currently required to be set to yes
BUILD_CONFORMANCE_TEST ?=yes
# Enable GCOV tool for code coverage
GCOV_ENABLED ?= 0

# arm-tidl builds against QNX SDP 7.1.0; set before tools_path.mak so
# sdk_builder's ?= 800 default is a no-op.
export QNX_SDP_VERSION ?= 710
PSDK_PATH ?= $(PSDK_INSTALL_PATH)
# To make sure if some other makefile includes arm-tidl/config.mk then they should define ARM_TIDL_PATH as happening in tiovx_kernels/makefile 
ARM_TIDL_PATH ?= $(ALGBASE_PATH)
include $(ARM_TIDL_PATH)/tools_path.mak

ifeq ($(TARGET_PLATFORM), PC)
# Below flag can be enabled in case opencv dependencies are present
BUILD_WITH_OPENCV    ?= 0
endif

#Compiles code for a particular C7x mode
C7x_HOSTEMU_COMPILER_STRING:=
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
C7x_HOSTEMU_COMPILER_STRING:=-MMA2_256
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
C7x_HOSTEMU_COMPILER_STRING:=-MMA3_1024
endif
