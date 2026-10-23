#
# Copyright (c) {2015 - 2018} Texas Instruments Incorporated
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

ifdef SystemRoot

PSDK_PATH ?= C:\ti
PSDK_BUILDER_PATH ?= $(PSDK_PATH)\sdk_builder

include $(PSDK_BUILDER_PATH)\tools_path.mak

else

PSDK_PATH ?= $(abspath ..)
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder

include $(PSDK_BUILDER_PATH)/tools_path.mak

endif

SHOW_COMMANDS       ?= 0
UTILS_PATH          ?= C:\\ti\\ccs930\\ccs\\utils\\cygwin

ifdef SystemRoot
PDK_INSTALL_PATH    ?=$(PSDK_PATH)\pdk\
CONCERTO_ROOT       ?=$(PSDK_BUILDER_PATH)\concerto
IVISION_PATH        ?=$(PSDK_PATH)\ivision
else
PDK_INSTALL_PATH    ?=$(PSDK_PATH)/pdk/
CONCERTO_ROOT       ?=$(PSDK_BUILDER_PATH)/concerto
IVISION_PATH        ?=$(PSDK_PATH)/ivision
endif

## ADALG specific flags
# Supported C66, C71, C7120
TARGET_CPU                     ?= C66
TARGET_PLATFORM                ?= TI_DEVICE
C7X_CORE_ID                    ?= 1
SOC                            ?= j721e
# Library and test application is always built using standalone make files by default, however it can be build using concerto by enabling below flag. Library build from concerto may not be funcitonal, test application is functional
# should be used for experimental purpose only
#TIADALG_BUILD                 ?= CONCERTO

# Default RTOS is FREERTOS
# Supported values: FREERTOS, NOOS. NOOS is only for PC mode, and it may not run on platform even though build may pass. For NOOS mode to run on
RTOS ?= FREERTOS
