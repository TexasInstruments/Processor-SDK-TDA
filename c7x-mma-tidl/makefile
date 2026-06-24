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
TIDL_BUILD_RELAY_IMPORT_LIB ?= 1
TIDL_BUILD_TFLITE_IMPORT_LIB ?= 1
TIDL_BUILD_ONNX_IMPORT_LIB ?= 1

ALGBASE_PATH ?= $(abspath ./)
PSDK_INSTALL_PATH ?= $(abspath ../)
TIDL_PATH ?=$(abspath ./)

include $(ALGBASE_PATH)/makerules/config.mk

$(info ======== CHECKING FOR DEPENDENT MAKEFILES ========)

ifdef SystemRoot
	MAKE = gmake
	RM_CMD = del /Q
	IT_RUN_SCRIP = import_all_models.bat
	TIDL_EXE = PC_dsp_test_dl_algo.out.exe
	TIDL_RUN_DIR = ti_dl\test
else
	ifeq ($(shell uname), Linux)
	  RM_CMD = rm -rf
	  MAKE = make
	  IT_RUN_SCRIP = ./import_all_models.sh
	  TIDL_CONF_IMPORT = ./import_conf_models.sh
      TIDL_RUN_DIR = ti_dl/test
	  TIDL_EXE = ./PC_dsp_test_dl_algo.out
	  TIDL_RT_EXE = ../../arm-tidl/rt/out/PC/x86_64/LINUX/$(TARGET_BUILD)/PC_dsp_test_dl_algo_host_rt.out
	  LD_LIBRARY_PATH := $(ALGBASE_PATH)/arm-tidl/rt/out/PC/x86_64/LINUX/$(TARGET_BUILD)
	endif
endif

ifneq ("$(wildcard ./ti_dl/algo/makefile)","")
    TIDL_ALGO_FILE_EXISTS = 1
    $(info [CHECK] ./ti_dl/algo/makefile - FOUND)
else
    TIDL_ALGO_FILE_EXISTS = 0
    $(info [CHECK] ./ti_dl/algo/makefile - NOT FOUND)
endif

ifneq ("$(wildcard ./tidl_priv/source/makefile)","")
    TIDL_PRIV_FILE_EXISTS = 1
    $(info [CHECK] ./tidl_priv/source/makefile - FOUND)
else
    TIDL_PRIV_FILE_EXISTS = 0
    $(info [CHECK] ./tidl_priv/source/makefile - NOT FOUND)
endif

ifneq ("$(wildcard ./tidl_ref/source/makefile)","")
    TIDL_REF_FILE_EXISTS = 1
    $(info [CHECK] ./tidl_ref/source/makefile - FOUND)
else
    TIDL_REF_FILE_EXISTS = 0
    $(info [CHECK] ./tidl_ref/source/makefile - NOT FOUND)
endif

ifneq ("$(wildcard ./tidl_ref/source/avx/makefile)","")
    TIDL_REF_AVX_FILE_EXISTS = 1
    $(info [CHECK] ./tidl_ref/source/avx/makefile - FOUND)
else
    TIDL_REF_AVX_FILE_EXISTS = 0
    $(info [CHECK] ./tidl_ref/source/avx/makefile - NOT FOUND)
endif

ifneq ("$(wildcard ./tidl-kernels/build_all.sh)","")
    TIDL_KERNELS_FILE_EXISTS = 1
    $(info [CHECK] ./tidl-kernels/build_all.sh - FOUND)
else
    TIDL_KERNELS_FILE_EXISTS = 0
    $(info [CHECK] ./tidl-kernels/build_all.sh - NOT FOUND)
endif

ifneq ("$(wildcard ./ti_dl/utils/perfsim/makefile)","")
    TIDL_PERFSIM_FILE_EXISTS = 1
    $(info [CHECK] ./ti_dl/utils/perfsim/makefile - FOUND)
else
    TIDL_PERFSIM_FILE_EXISTS = 0
    $(info [CHECK] ./ti_dl/utils/perfsim/makefile - NOT FOUND)
endif

ifneq ("$(wildcard ./ti_dl/utils/tidlModelSecure/makefile)","")
    TIDL_MS_FILE_EXISTS = 1
    $(info [CHECK] ./ti_dl/utils/tidlModelSecure/makefile - FOUND)
else
    TIDL_MS_FILE_EXISTS = 0
    $(info [CHECK] ./ti_dl/utils/tidlModelSecure/makefile - NOT FOUND)
endif

ifneq ("$(wildcard ./ti_dl/utils/tidlModelGraphviz/makefile_runtimes)","")
    TIDL_GV_RT_FILES_EXIST = 1
    $(info [CHECK] ./ti_dl/utils/tidlModelGraphviz/makefile_runtimes - FOUND)
else
    TIDL_GV_RT_FILES_EXIST = 0
    $(info [CHECK] ./ti_dl/utils/tidlModelGraphviz/makefile_runtimes - NOT FOUND)
endif

$(info ======== MAKEFILE CHECKS COMPLETE ========)

#$(info TIDL_ALGO_FILE_EXISTS $(TIDL_ALGO_FILE_EXISTS))
#$(info TIDL_PERFSIM_FILE_EXISTS $(TIDL_PERFSIM_FILE_EXISTS))

ifdef SystemRoot
all_targets: protos nc it tidl dt ms
else
all_targets: protos nc it tidl dt ms gv gv_rt
endif

all: all_targets

tidl_cuda:
	@echo .
ifeq ($(TIDL_ALGO_FILE_EXISTS) , 1)
	@echo ======== MAKING TIDL ALGO =================
	$(MAKE) -C ./ti_dl/algo -f makefile
	$(MAKE) -C ./ti_dl/custom -f makefile
endif
ifeq ($(TIDL_REF_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL REF =================
	$(MAKE) -C ./tidl_ref/source -f makefile
endif
ifeq ($(TIDL_PRIV_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL PRIV =================
	$(MAKE) -C ./tidl_priv/source -f makefile
endif
ifeq ($(TIDL_KERNELS_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL KERNELS =================
	cmake -B ./tidl-kernels/build ./tidl-kernels
	$(MAKE) -C ./tidl-kernels/build -f Makefile tidl-kernels
endif
#	@echo .
#	@echo ======== MAKING TIDL TEST =================
#	$(MAKE) -C ./ti_dl/test -f legacy_cuda_makefile

tidl_cuda_clean:
	@echo .
ifeq ($(TIDL_ALGO_FILE_EXISTS) , 1)
	@echo ======== CLEANING TIDL ALGO =================
	$(MAKE) -C ./ti_dl/algo -f makefile clean
	$(MAKE) -C ./ti_dl/custom -f makefile clean
endif
ifeq ($(TIDL_REF_FILE_EXISTS) , 1)
	@echo .
	@echo ======== CLEANING TIDL REF =================
	$(MAKE) -C ./tidl_ref/source -f makefile clean
endif
ifeq ($(TIDL_PRIV_FILE_EXISTS) , 1)
	@echo .
	@echo ======== CLEANING TIDL PRIV =================
	$(MAKE) -C ./tidl_priv/source -f makefile clean
endif
ifeq ($(TIDL_KERNELS_FILE_EXISTS) , 1)
	@echo .
	@echo ======== CLEANING TIDL KERNELS =================
	$(MAKE) -C ./tidl-kernels/build -f Makefile clean
	rm -rf ./tidl-kernels/build
endif
#	@echo .
#	@echo ======== CLEANING TIDL TEST =================
#	$(MAKE) -C ./ti_dl/test -f legacy_cuda_makefile clean

tidl_algo:
ifeq ($(TIDL_ALGO_FILE_EXISTS) , 1)
	@echo ======== MAKING TIDL ALGO =================
	$(MAKE) -C ./ti_dl/algo -f makefile
endif

tidl_algo_clean:
ifeq ($(TIDL_ALGO_FILE_EXISTS) , 1)
	@echo ======== MAKING TIDL ALGO =================
	$(MAKE) -C ./ti_dl/algo -f makefile clean
endif

tidl_priv:
ifeq ($(TIDL_PRIV_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL PRIV =================
	$(MAKE) -C ./tidl_priv/source -f makefile
endif

tidl_priv_clean:
ifeq ($(TIDL_PRIV_FILE_EXISTS) , 1)
	@echo PRIV=YES!
	@echo ======== CLEANING TIDL PRIV =================
	$(MAKE) -C ./tidl_priv/source -f makefile clean
endif

tidl_ref:
ifeq ($(TIDL_REF_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL REF =================
	$(MAKE) -C ./tidl_ref/source -f makefile
endif
ifeq ($(TARGET_PLATFORM), PC)
ifeq ($(TIDL_REF_AVX_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL REF AVX =================
	$(MAKE) -C ./tidl_ref/source/avx -f makefile
endif
endif

tidl_ref_clean:
ifeq ($(TIDL_REF_FILE_EXISTS) , 1)
	@echo .
	@echo ======== CLEANING TIDL REF =================
	$(MAKE) -C ./tidl_ref/source -f makefile clean
endif
ifeq ($(TARGET_PLATFORM), PC)
ifeq ($(TIDL_REF_AVX_FILE_EXISTS) , 1)
	@echo .
	@echo ======== CLEANING TIDL REF AVX =================
	$(MAKE) -C ./tidl_ref/source/avx -f makefile clean
endif
endif

tidl_kernels:
ifeq ($(TIDL_KERNELS_FILE_EXISTS) , 1)
	@echo .
	@echo ======== MAKING TIDL KERNELS =================
ifeq ($(TARGET_PLATFORM) , PC)
	cmake -B ./tidl-kernels/x86_64_build ./tidl-kernels -DSOC=$(TARGET_SOC) -DBUILD_ALL_KERNELS=OFF $(TIDL_SUPPORTED_KERNEL_FLAGS) -DPSDK_INSTALL_PATH=$(PSDK_INSTALL_PATH) -DPERF_MODEL=1 -DBUILD_TESTS=OFF -DHOST_EMU=ON -DCMAKE_BUILD_TYPE=$(TARGET_BUILD) -DINSTALL_PATH=$(ALGBASE_PATH)/ti_dl -DGCOV_ENABLED=$(GCOV_ENABLED) -DMMALIB_PATH=$(MMALIB_PATH)
	$(MAKE) -C ./tidl-kernels/x86_64_build -f Makefile install
else
	cmake -B ./tidl-kernels/dsp_build ./tidl-kernels -DSOC=$(TARGET_SOC) -DBUILD_ALL_KERNELS=OFF $(TIDL_SUPPORTED_KERNEL_FLAGS) -DPSDK_INSTALL_PATH=$(PSDK_INSTALL_PATH) -DBUILD_TESTS=OFF -DHOST_EMU=OFF -DCMAKE_BUILD_TYPE=$(TARGET_BUILD) -DINSTALL_PATH=$(ALGBASE_PATH)/ti_dl -DCODE_COVERAGE_ENABLED_FOR_TIDL=$(CODE_COVERAGE_ENABLED_FOR_TIDL) -DMMALIB_PATH=$(MMALIB_PATH)
	$(MAKE) -C ./tidl-kernels/dsp_build -f Makefile install
endif
endif

tidl_kernels_clean:
ifeq ($(TIDL_KERNELS_FILE_EXISTS) , 1)
	@echo .
	@echo ======== CLEANING TIDL KERNELS =================
ifeq ($(TARGET_PLATFORM) , PC)
	@if [ -d "./tidl-kernels/x86_64_build" ]; then \
		$(MAKE) -C ./tidl-kernels/x86_64_build -f Makefile clean; \
		rm -rf ./tidl-kernels/x86_64_build; \
	fi
else
	@if [ -d "./tidl-kernels/dsp_build" ]; then \
		$(MAKE) -C ./tidl-kernels/dsp_build -f Makefile clean; \
		rm -rf ./tidl-kernels/dsp_build; \
	fi
endif
endif

tidl_lib:tidl_algo tidl_priv tidl_ref custom tidl_kernels
	@echo .
	@echo ======== MAKING TIDL AND CUSTOM LIBRARIES =================

tidl_lib_clean:tidl_algo_clean tidl_priv_clean tidl_ref_clean custom_clean tidl_kernels_clean
	@echo .
	@echo ======== CLEANING TIDL AND CUSTOM LIBRARIES =================

tidl_tiovx_dsp_kernels:
	@echo .
	@echo ======== MAKING TIDL TIOVX DSP KERNELS =================
	$(MAKE) -C ./tiovx_kernels -f makefile all

tidl_tiovx_dsp_kernels_clean:
	@echo .
	@echo ======== CLEANING TIDL TIOVX DSP KERNELS =================
	$(MAKE) -C ./tiovx_kernels -f makefile clean

tidl: tidl_lib tidl_tiovx_dsp_kernels
	@echo .
	@echo ======== MAKING TIDL TEST =================
	$(MAKE) -C ./ti_dl/test -f makefile final_install

tidl_clean: tidl_lib_clean tidl_tiovx_dsp_kernels_clean
	@echo .
	@echo ======== CLEANING TIDL TEST =================
	$(MAKE) -C ./ti_dl/test -f makefile clean

rt:
	@echo .
	@echo ======== MAKING TIDL RT =================
	$(MAKE) -C ./arm-tidl -f makefile all

rt_clean:
	@echo .
	@echo ======== CLEANING TIDL RT =================
	$(MAKE) -C ./arm-tidl -f makefile clean

tidl_rt:
	@echo .
	@echo ======== MAKING TIDL RT =================
	$(MAKE) -C ./arm-tidl -f makefile rt

tidl_rt_clean:
	@echo .
	@echo ======== CLEANING TIDL RT =================
	$(MAKE) -C ./arm-tidl -f makefile rt_clean

nc:
	@echo .
ifeq ($(TIDL_PERFSIM_FILE_EXISTS) , 1)
	@echo ======== MAKING TIDL PERFSIM =================
	$(MAKE) -C ./ti_dl/utils/perfsim -f makefile
endif
nc_clean:
	@echo .
ifeq ($(TIDL_PERFSIM_FILE_EXISTS) , 1)
	@echo ======== CLEANING TIDL PERFSIM ===============
	$(MAKE) -C ./ti_dl/utils/perfsim -f makefile clean
	cd ti_dl/utils/perfsim && $(RM_CMD) ti_cnnperfsim*
endif

ms:
	@echo .
ifeq ($(TIDL_MS_FILE_EXISTS) , 1)
	@echo ======== MAKING MODEL SECURITY =================
	$(MAKE) -C ./ti_dl/utils/tidlModelSecure -f makefile
endif
ms_clean:
	@echo .
ifeq ($(TIDL_MS_FILE_EXISTS) , 1)
	@echo ======== CLEANING MODEL SECURITY ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelSecure -f makefile clean
	cd ti_dl/utils/tidlModelSecure && $(RM_CMD) tidl_model_secure*
endif

gv: dt
	@echo .
	@echo ======== MAKING GRAPH VIZ TOOL =================
	$(MAKE) -C ./ti_dl/utils/tidlModelGraphviz -f makefile

gv_clean: dt_clean
	@echo .
	@echo ======== CLEANING GRAPH VIZ  TOOL ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelGraphviz -f makefile clean

gv_rt:
	@echo .
ifeq ($(TIDL_GV_RT_FILES_EXIST) , 1)
	@echo ======== MAKING GRAPH VIZ TOOL =================
	$(MAKE) -C ./ti_dl/utils/tidlModelGraphviz -f makefile_runtimes
endif
gv_rt_clean:
	@echo .
ifeq ($(TIDL_GV_RT_FILES_EXIST) , 1)
	@echo ======== CLEANING GRAPH VIZ  TOOL ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelGraphviz -f makefile_runtimes clean
endif
dt:
	@echo .
	@echo ======== MAKING TIDL DUMP TOOL =================
	$(MAKE) -C ./ti_dl/utils/tidlModelDump -f makefile

dt_clean:
	@echo .
	@echo ======== CLEANING TIDL DUMP TOOL ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelDump -f makefile clean


it_lib: protos
	@echo .
	@echo ======== MAKING CUSTOM IMPORT LIBS =================
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared_custom
	@echo .
	@echo ======== MAKING IMPORT LIBS =================
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared
ifeq ($(TIDL_BUILD_RELAY_IMPORT_LIB), 1)
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared tvm_relay
endif
ifeq ($(TIDL_BUILD_TFLITE_IMPORT_LIB), 1)
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared tflite_runtime
endif
ifeq ($(TIDL_BUILD_ONNX_IMPORT_LIB), 1)
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared onnx_runtime
endif

it: it_lib
	@echo .
	@echo ======== MAKING IMPORT TOOL =================
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_bin

tidl_safety_bin:
	@echo .
	@echo ============ OVERWRITE_COMPILED_MODEL =================
	$(MAKE) -C ./ti_dl/utils/testAutomation/tidl_safety -f makefile_overwrite

tidl_safety_clean:
	@echo .
	@echo ======== CLEANING OVERWRITE_MODEL_BIN ===============
	$(MAKE) -C ./ti_dl/utils/testAutomation/tidl_safety -f makefile_overwrite clean

it_lib_clean:
	@echo .
	@echo ======== CLEANING IMPORT LIBS ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared clean
	@echo .
	@echo ======== CLEANING CUSTOM IMPORT LIBS ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_shared_custom clean

it_clean: it_lib_clean
	@echo .
	@echo ======== CLEANING IMPORT TOOL ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_bin clean

protos:
	@echo .
	@echo ======== MAKING IMPORT TOOL PROTOS LIB =================
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_lib

protos_clean:
	@echo .
	@echo ======== CLEANING IMPORT TOOL PROTOS LIB ===============
	$(MAKE) -C ./ti_dl/utils/tidlModelImport -f makefile_lib clean

custom:
	@echo .
	@echo ======== MAKING CUSTOM LIB =================
	$(MAKE) -C ./ti_dl/custom -f makefile


custom_clean:
	@echo .
	@echo ======== CLEANING CUSTOM LIB ===============
	$(MAKE) -C ./ti_dl/custom -f makefile clean

.IGNORE: clean
ifdef SystemRoot
clean: nc_clean it_clean tidl_clean dt_clean
else
clean: nc_clean it_clean tidl_clean gv_clean
endif

.IGNORE: clean
cleanall: protos_clean nc_clean it_clean tidl_clean gv_clean gv_rt_clean dt_clean ms_clean

scrub: cleanall
	rm -rf ./ti_dl/lib
	rm -rf ./out
	rm -rf ./ti_dl/test/lib
	rm -rf ./ti_dl/test/out
	rm -rf ./arm-tidl/rt/lib
	rm -rf ./arm-tidl/rt/out
	rm -rf ./arm-tidl/onnxrt_ep/out
	rm -rf ./arm-tidl/onnxrt_ep/lib
	rm -rf ./arm-tidl/tfl_delegate/out
	rm -rf ./arm-tidl/tfl_delegate/lib
	rm -rf ./arm-tidl/tidlrt_ep/out
	rm -rf ./arm-tidl/tidlrt_ep/lib
	rm -rf ./arm-tidl/tiovx_kernels/out
	rm -rf ./arm-tidl/tiovx_kernels/lib
	rm -rf ./ti_dl/utils/tidlModelImport/lib/
	rm -rf ./ti_dl/utils/tidlModelImport/out/
	rm -rf ./ti_dl/utils/tidlModelSecure/out/
	rm -rf ./tiovx_kernels/lib
	rm -rf ./tiovx_kernels/out
	rm -f ./ti_dl/test/PC_dsp_test_dl_algo.out
	rm -f ./ti_dl/test/j7-c71_0-fw

it_run:
	cd ti_dl/utils/tidlModelImport && $(IT_RUN_SCRIP) &&  cd ../../../

tidl_conf_models_import:
	cd ti_dl/utils/tidlModelImport && $(TIDL_CONF_IMPORT) &&  cd ../../../

tidl_run:
	cd ti_dl/test && $(TIDL_EXE)  &&  cd ../../

tidl_rt_run: export LD_LIBRARY_PATH:=$(LD_LIBRARY_PATH)
tidl_rt_run:
	cd ti_dl/test && $(TIDL_RT_EXE)  &&  cd ../../

run: it_run tidl_run

dss_run:
	cd ti_dl/utils/targetTest && C:\ti\ccs910\ccs\ccs_base\scripting\bin\dss.bat launch_tidl_v2.js

tidl_tiovx_arm_kernels:
	@echo .
	@echo ======== MAKING TIDL TIOVX ARM KERNELS =================
	$(MAKE) -C ./arm-tidl -f makefile tidl_tiovx_arm_kernels

tidl_tiovx_arm_kernels_clean:
	@echo .
	@echo ======== CLEANING TIDL TIOVX ARM KERNELS =================
	$(MAKE) -C ./arm-tidl -f makefile tidl_tiovx_arm_kernels_clean

install:
ifndef TIDL_PATH
	$(error TIDL_PATH is not defined. Please set TIDL_PATH before running install)
else
	@echo "Installing to $(TIDL_PATH)...";
	@if [ -d "$(TIDL_PATH)" ]; then \
		mkdir -p $(TIDL_PATH)/ti_dl/lib/; \
		mkdir -p $(TIDL_PATH)/ti_dl/inc/; \
		mkdir -p $(TIDL_PATH)/tiovx_kernels/lib/; \
		cp -rfL ./ti_dl/lib/* $(TIDL_PATH)/ti_dl/lib/ 2>/dev/null || :; \
		cp -rfL ./ti_dl/inc/* $(TIDL_PATH)/ti_dl/inc/ 2>/dev/null || :; \
		cp -rfL ./tiovx_kernels/lib/* $(TIDL_PATH)/tiovx_kernels/lib/ 2>/dev/null || :; \
		echo "TIDL Libraries and Headers Installation complete"; \
	else \
		echo "Error: TIDL_PATH '$(TIDL_PATH)' does not exist. Please provide a valid directory path."; \
		exit 1; \
	fi
endif

install_tidl_tools:
ifeq ($(TARGET_PLATFORM), PC)
	@echo "Copying tidl_tools tarball to $(TIDL_PATH)/tidl_tools/$(TARGET_SOC)...";
	@if [ ! -d "$(TIDL_PATH)" ]; then \
		echo "Error: TIDL_PATH '$(TIDL_PATH)' does not exist. Please provide a valid directory path."; \
		exit 1; \
	fi

	@if [ ! -d "./tidl_tools" ]; then \
		echo "Error: tidl_tools directory does not exist."; \
		exit 1; \
	fi
	@mkdir -p $(TIDL_PATH)/tidl_tools/$(TARGET_SOC);
	@tar -h -czf $(TIDL_PATH)/tidl_tools/$(TARGET_SOC)/tidl_tools.tar.gz -C ./tidl_tools .;
	@echo "TIDL_TOOLS Installation complete";
endif

.PHONY: nc it tidl ms nc_clean it_clean tidl_clean ms_clean it_clean protos_clean protos it_run tidl_run gv gv_clean gv_rt gv_rt_clean dt dt_clean tidl_tiovx_arm_kernels tidl_tiovx_dsp_kernels tidl_tiovx_arm_kernels_clean tidl_tiovx_dsp_kernels_clean scrub tidl_lib tidl_lib_clean tidl_algo tidl_algo_clean tidl_priv tidl_priv_clean tidl_ref tidl_ref_clean tidl_kernels tidl_kernels_clean custom custom_clean install install_tidl_tools




