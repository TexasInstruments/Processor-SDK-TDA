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
# First Push to git.ti.com - 3
ifeq ($(shell uname), Linux)
	MAKE = make
endif

ALGBASE_PATH      ?= $(abspath ./)
PSDK_INSTALL_PATH ?= $(abspath ../../)
PSDK_BUILDER_PATH ?= $(PSDK_INSTALL_PATH)/sdk_builder
include $(ALGBASE_PATH)/config.mk

all: rt tfl_delegate onnxrt_ep tidlrt_ep

rt:
	@echo .
	@echo ======== MAKING TIDL RT =================
	$(MAKE) -C ./rt -f makefile all

rt_clean:
	@echo .
	@echo ======== CLEANING TIDL RT =================
	$(MAKE) -C ./rt -f makefile clean

rt_scrub:
	@echo .
	@echo ======== Scrubbing TIDL RT =================
	rm -rf ./rt/out ./rt/lib


tfl_delegate: rt
	@echo .
	@echo ======== MAKING TIDL TFLITE DELEGATE =================
	$(MAKE) -C ./tfl_delegate -f makefile all

tfl_delegate_clean:
	@echo .
	@echo ======== CLEANING TIDL TFLITE DELEGATE =================
	$(MAKE) -C ./tfl_delegate -f makefile clean

tfl_delegate_scrub:
	@echo .
	@echo ======== Scrubbing TIDL TFLITE DELEGATE  =================
	rm -rf ./tfl_delegate/out ./tfl_delegate/lib

onnxrt_ep: rt
	@echo .
	@echo ======== MAKING TIDL ONNXRT EP =================
	$(MAKE) -C ./onnxrt_ep -f makefile all

onnxrt_ep_clean:
	@echo .
	@echo ======== CLEANING TIDL ONNXRT EP =================
	$(MAKE) -C ./onnxrt_ep -f makefile clean

onnxrt_ep_scrub:
	@echo .
	@echo ======== Scrubbing TIDL TFLITE DELEGATE  =================
	rm -rf ./onnxrt_ep/out ./onnxrt_ep/lib

tidlrt_ep: rt
	@echo .
	@echo ======== MAKING TIDL RT EP =================
	$(MAKE) -C ./tidlrt_ep -f makefile all

tidlrt_ep_clean:
	@echo .
	@echo ======== CLEANING TIDL RT EP =================
	$(MAKE) -C ./tidlrt_ep -f makefile clean

tidlrt_ep_scrub:
	@echo .
	@echo ======== Scrubbing TIDL RT EP  =================
	rm -rf ./tidlrt_ep/out ./tidlrt_ep/lib

tidl_tiovx_arm_kernels:
	@echo .
	@echo ======== MAKING TIDL TIOVX ARM KERNELS =================
	$(MAKE) -C ./tiovx_kernels -f makefile all

tidl_tiovx_arm_kernels_clean:
	@echo .
	@echo ======== CLEANING TIDL TIOVX ARM KERNELS =================
	$(MAKE) -C ./tiovx_kernels -f makefile clean

tidl_tiovx_arm_kernels_scrub:
	@echo .
	@echo ======== SCRUBBING TIDL TIOVX ARM KERNELS  =================
	rm -rf ./tiovx_kernels/out ./tiovx_kernels/lib

tidl_tiovx_arm_kernels_install:
ifndef TIDL_PATH
	$(error TIDL_PATH is not defined. Please set TIDL_PATH before running install)
else
	@echo "Installing TIDL TIOVX ARM KERNELS to $(TIDL_PATH)..."
	@if [ -d "$(TIDL_PATH)" ]; then \
		mkdir -p $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tiovx_kernels/out/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tiovx_kernels/include/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tiovx_kernels/tidl/include/; \
		cp -rfL ./tiovx_kernels/lib/* $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/ 2>/dev/null || :; \
		cp -rfL ./tiovx_kernels/out/* $(TIDL_PATH)/arm-tidl/tiovx_kernels/out/ 2>/dev/null || :; \
		cp -rfL ./tiovx_kernels/include/* $(TIDL_PATH)/arm-tidl/tiovx_kernels/include/ 2>/dev/null || :; \
		cp -rfL ./tiovx_kernels/tidl/include/* $(TIDL_PATH)/arm-tidl/tiovx_kernels/tidl/include/ 2>/dev/null || :; \
		echo "TIDL TIOVX ARM KERNELS installation complete"; \
	else \
		echo "Error: TIDL_PATH '$(TIDL_PATH)' does not exist. Please provide a valid directory path."; \
		exit 1; \
	fi
endif

install: tidl_tiovx_arm_kernels_install
ifndef TIDL_PATH
	$(error TIDL_PATH is not defined. Please set TIDL_PATH before running install)
else
	@echo "Installing to $(TIDL_PATH)..."
	@if [ -d "$(TIDL_PATH)" ]; then \
		mkdir -p $(TIDL_PATH)/arm-tidl/rt/inc/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/rt/lib/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/rt/out/; \
		cp -rfL ./rt/inc/* $(TIDL_PATH)/arm-tidl/rt/inc/ 2>/dev/null || :; \
		cp -rfL ./rt/lib/* $(TIDL_PATH)/arm-tidl/rt/lib/ 2>/dev/null || :; \
		cp -rfL ./rt/out/* $(TIDL_PATH)/arm-tidl/rt/out/ 2>/dev/null || :; \
		mkdir -p $(TIDL_PATH)/arm-tidl/onnxrt_ep/lib/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/onnxrt_ep/out/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tidlrt_ep/lib/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tidlrt_ep/out/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tfl_delegate/lib/; \
		mkdir -p $(TIDL_PATH)/arm-tidl/tfl_delegate/out/; \
		cp -rfL ./onnxrt_ep/lib/* $(TIDL_PATH)/arm-tidl/onnxrt_ep/lib/ 2>/dev/null || :; \
		cp -rfL ./onnxrt_ep/out/* $(TIDL_PATH)/arm-tidl/onnxrt_ep/out/ 2>/dev/null || :; \
		cp -rfL ./tidlrt_ep/lib/* $(TIDL_PATH)/arm-tidl/tidlrt_ep/lib/ 2>/dev/null || :; \
		cp -rfL ./tidlrt_ep/out/* $(TIDL_PATH)/arm-tidl/tidlrt_ep/out/ 2>/dev/null || :; \
		cp -rfL ./tfl_delegate/lib/* $(TIDL_PATH)/arm-tidl/tfl_delegate/lib/ 2>/dev/null || :; \
		cp -rfL ./tfl_delegate/out/* $(TIDL_PATH)/arm-tidl/tfl_delegate/out/ 2>/dev/null || :; \
		echo "Installation complete"; \
	else \
		echo "Error: TIDL_PATH '$(TIDL_PATH)' does not exist. Please provide a valid directory path."; \
		exit 1; \
	fi
endif

scrub: rt_scrub tfl_delegate_scrub onnxrt_ep_scrub tidlrt_ep_scrub tidl_tiovx_arm_kernels_scrub

.IGNORE: clean
clean: rt_clean tfl_delegate_clean onnxrt_ep_clean tidlrt_ep_clean tidl_tiovx_arm_kernels_clean

.PHONY: rt tfl_delegate onnxrt_ep tidlrt_ep tidl_tiovx_arm_kernels rt_clean tfl_delegate_clean onnxrt_ep_clean tidlrt_ep_clean tidl_tiovx_arm_kernels_clean tidl_tiovx_arm_kernels_install install
