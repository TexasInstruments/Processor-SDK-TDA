
include tools_path.mak

# Edit below file to change default build options
include build_flags.mak

# Project specific build defs (don't change across different combos):
BUILD_DEFS :=
ifneq ($(APP_CONFIG_BASE_PATH),)
    BUILD_DEFS += APP_CONFIG_BASE_PATH
endif

BUILD_DEFS += $(SOC_DEF)
BUILD_DEFS += $(ECU_BUILD_DEF)
BUILD_DEFS += $(foreach CORE,$(ENABLED_IPC_CORES),$(CORE))

ifeq ($(SOC), am62a)
    BUILD_DEFS += $(TISDK_IMAGE)
endif

ifeq ($(LDRA_COVERAGE_ENABLED_IMAGING), yes)
    BUILD_DEFS += LDRA_COVERAGE
endif

DIRECTORIES :=

ifeq ($(SOC),am62a)
    DIRECTORIES += utils/app_init
    DIRECTORIES += utils/sciserver
    DIRECTORIES += utils/mpu
    DIRECTORIES += platform/$(SOC)
    DIRECTORIES += modules
    DIRECTORIES += apps/basic_demos/app_c7x_kernel
    DIRECTORIES += apps/basic_demos/app_fd_exchange
    DIRECTORIES += apps/tests/app_vx_conformance
    DIRECTORIES += apps/tests/app_vx_conformance_core
    DIRECTORIES += apps/tests/app_vx_conformance_hwa
    DIRECTORIES += apps/tests/app_vx_conformance_tidl

    ifeq ($(BUILD_EDGEAI), no)
        DIRECTORIES += apps/tests/app_vx_conformance_video_io
    endif
    DIRECTORIES += apps/utilities

    DIRECTORIES += kernels/fileio
    DIRECTORIES += kernels/img_proc

    #Note: For QNX use case, include the below directories irrespective of adas of edgeai image
    ifeq ($(BUILD_QNX_MPU),yes)
        ifeq ($(BUILD_EDGEAI), yes)
            DIRECTORIES += apps/basic_demos/app_multi_cam
            DIRECTORIES += utils/codec_wrapper
            DIRECTORIES += utils/openmax_wrapper
            DIRECTORIES += apps/basic_demos/app_multi_cam_codec
        endif
        DIRECTORIES += utils/draw2d
        DIRECTORIES += utils/grpx
        DIRECTORIES += apps/basic_demos/app_single_cam
        DIRECTORIES += apps/dl_demos/app_tidl_cam
        DIRECTORIES += apps/dl_demos/app_tidl
        DIRECTORIES += apps/dl_demos/app_tidl_od
        DIRECTORIES += apps/dl_demos/app_tidl_od_cam
    endif

    #Note: For Linux use case, include the below directories only for adas use case
    ifeq ($(BUILD_LINUX_MPU)$(BUILD_EDGEAI),yesno)
        DIRECTORIES += utils/drm_wrapper
        DIRECTORIES += apps/basic_demos/app_drm_test
        DIRECTORIES += utils/draw2d
        DIRECTORIES += utils/grpx
        DIRECTORIES += apps/basic_demos/app_single_cam
        DIRECTORIES += apps/dl_demos/app_tidl
        DIRECTORIES += apps/dl_demos/app_tidl_cam
        DIRECTORIES += apps/dl_demos/app_tidl_od
        DIRECTORIES += apps/dl_demos/app_tidl_od_cam
    endif
else

    ifeq ($(CTOOLS_BUILD_ENABLED), yes)
        ifeq ($(SOC),$(filter $(SOC),j722s j721e j721s2 j784s4 j742s2))
            DIRECTORIES += utils/ctools_wrapper
            BUILD_DEFS += APP_CTOOLS_ENABLED
        endif
    endif

    ifneq ($(SOC),tda54)
        DIRECTORIES += utils
        DIRECTORIES += kernels
        DIRECTORIES += applibs
    endif

    DIRECTORIES += modules
    DIRECTORIES += platform/$(SOC)

    ifeq ($(BUILD_EDGEAI),no)
        ifeq ($(SOC),tda54)
            DIRECTORIES += kernels/img_proc
            DIRECTORIES += kernels/fileio
            DIRECTORIES += utils/app_init
            DIRECTORIES += utils/app_init_vdk
            DIRECTORIES += utils/grpx
            DIRECTORIES += utils/draw2d
            DIRECTORIES += utils/dss
            DIRECTORIES += apps/utilities/app_remote_log
            DIRECTORIES += apps/utilities/app_ipc
            DIRECTORIES += apps/utilities/app_mem
            DIRECTORIES += apps/tests/app_vx_conformance_tidl
            DIRECTORIES += apps/tests/app_vx_conformance_hwa
            DIRECTORIES += apps/tests/app_vx_conformance_core
            DIRECTORIES += apps/tests/app_vx_conformance_video_io
            DIRECTORIES += apps/dl_demos/app_tidl
            DIRECTORIES += apps/dl_demos/app_tidl_od
            DIRECTORIES += apps/dl_demos/app_tidl_bev
        else
            DIRECTORIES += apps
        endif
    endif
endif

TARGET_COMBOS :=

ifeq ($(BUILD_TARGET_MODE),yes)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
    ifeq ($(BUILD_ISA_R5F),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:3:debug:TIARMCGT_LLVM
    endif
    ifeq ($(BUILD_ISA_R52P),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R52P:3:debug:TIARMCGT_LLVM
    endif
    ifeq ($(BUILD_ISA_M55),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):M55:3:debug:TIARMCGT_LLVM
    endif
    ifeq ($(BUILD_ISA_C6x),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:2:debug:CGT6X
    endif
    ifeq ($(BUILD_ISA_C7x),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:debug:CGT7X
    endif
    ifeq ($(BUILD_ISA_MPU),yes)
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:debug:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:debug:GCC_QNX_ARM
        endif
    endif
  endif

  ifeq ($(PROFILE), $(filter $(PROFILE), release all))
    ifeq ($(BUILD_ISA_R5F),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:3:release:TIARMCGT_LLVM
    endif
    ifeq ($(BUILD_ISA_R52P),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R52P:3:release:TIARMCGT_LLVM
    endif
    ifeq ($(BUILD_ISA_M55),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):M55:3:release:TIARMCGT_LLVM
    endif
    ifeq ($(BUILD_ISA_C6x),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:2:release:CGT6X
    endif
    ifeq ($(BUILD_ISA_C7x),yes)
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:release:CGT7X
    endif
    ifeq ($(BUILD_ISA_MPU),yes)
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:release:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:release:GCC_QNX_ARM
        endif
    endif
  endif
endif

ifeq ($(BUILD_EMULATION_MODE),yes)
  ifeq ($(OS),Windows_NT)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            TARGET_COMBOS += PC:WINDOWS:x86_64:1:debug:GCC_WINDOWS
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            TARGET_COMBOS += PC:WINDOWS:X86:1:debug:GCC_WINDOWS
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            TARGET_COMBOS += PC:WINDOWS:x86_64:1:release:GCC_WINDOWS
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            TARGET_COMBOS += PC:WINDOWS:X86:1:release:GCC_WINDOWS
        endif
    endif
  else
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            ifeq ($(BUILD_IMAGING_OPENACC), yes)
                ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4 j742s2))
                    TARGET_COMBOS += PC:LINUX:x86_64:1:debug:NVCPP_LINUX
                else
                    TARGET_COMBOS += PC:LINUX:x86_64:1:debug:GCC_LINUX
                endif
            else
                TARGET_COMBOS += PC:LINUX:x86_64:1:debug:GCC_LINUX
            endif
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            ifeq ($(BUILD_IMAGING_OPENACC), yes)
                ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4 j742s2))
                    TARGET_COMBOS += PC:LINUX:X86:1:debug:NVCPP_LINUX
                else
                    TARGET_COMBOS += PC:LINUX:X86:1:debug:GCC_LINUX
                endif
            else
                TARGET_COMBOS += PC:LINUX:X86:1:debug:GCC_LINUX
            endif
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), x86_64 all))
            ifeq ($(BUILD_IMAGING_OPENACC), yes)
                ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4 j742s2))
                    TARGET_COMBOS += PC:LINUX:x86_64:1:release:NVCPP_LINUX
                else
                    TARGET_COMBOS += PC:LINUX:x86_64:1:release:GCC_LINUX
                endif
            else
                TARGET_COMBOS += PC:LINUX:x86_64:1:release:GCC_LINUX
            endif
        endif
        ifeq ($(BUILD_EMULATION_ARCH), $(filter $(BUILD_EMULATION_ARCH), X86 all))
            ifeq ($(BUILD_IMAGING_OPENACC), yes)
                ifeq ($(SOC), $(filter $(SOC), j721s2 j784s4 j742s2))
                    TARGET_COMBOS += PC:LINUX:X86:1:release:NVCPP_LINUX
                else
                    TARGET_COMBOS += PC:LINUX:X86:1:release:GCC_LINUX
                endif
            else
                TARGET_COMBOS += PC:LINUX:X86:1:release:GCC_LINUX
            endif
        endif
    endif
  endif
endif

CONCERTO_ROOT ?= $(PSDK_BUILDER_PATH)/concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak

# Project specific rules

.PHONY: all check_ipc_cores

all:

check_ipc_cores:
	$(call CHECK_IPC_CORES,$(VISION_APPS_PATH),$(ENABLED_IPC_CORES))

doxy_docs:
	-$(Q)$(MKDIR) docs/user_guide/ $(QUIET) || true
	-$(Q)$(COPY) tools/3d_calibration_tool/PSDK_RTOS_UserGuide_3D_SurroundView_Manual_CalibTool.pdf docs/user_guide/
	-$(Q)$(COPY) tools/3d_calibration_tool/poster_calib_chart.pdf docs/user_guide/
	$(DOXYGEN) internal_docs/doxy_cfg_user_guide/user_guide_linux_$(SOC).cfg 2> internal_docs/doxy_cfg_user_guide/doxy_warnings.txt
	-$(Q)$(COPY) internal_docs/doxy_cfg_user_guide/theme/*.png docs/user_guide/
	$(COPY) vision_apps_release_notes_$(SOC).html vision_apps_release_notes.html

doxy_design_docs:
	$(DOXYGEN) internal_docs/doxy_cfg_design/design_guide.cfg 2> internal_docs/doxy_cfg_design/doxy_warnings.txt

doxy_datasheet_docs:
	-$(Q)$(MKDIR) docs/datasheet/ $(QUIET) || true
	$(DOXYGEN) internal_docs/doxy_cfg_datasheet/datasheet_$(SOC).cfg 2> internal_docs/doxy_cfg_datasheet/doxy_warnings.txt

define CHECK_IPC_CORES
    $(eval core_cfg := $(1)/.build_core.bak)
    $(eval core_cfg_temp := $(1)/.build_core_temp.bak)
    @if [ ! -f $(core_cfg) ]; then \
        touch $(core_cfg); \
        for core in $(2); do \
            echo $$core >> $(core_cfg); \
        done; \
    else \
        touch $(core_cfg_temp); \
        for core in $(2); do \
            echo $$core >> $(core_cfg_temp); \
        done; \
        if ! diff -q $(core_cfg) $(core_cfg_temp) > /dev/null; then \
            touch $(VISION_APPS_PATH)/platform/$(SOC)/rtos/common/app_init.c; \
            touch $(VISION_APPS_PATH)/platform/$(SOC)/linux/app_init.c; \
            touch $(VISION_APPS_PATH)/platform/$(SOC)/qnx/app_init.c; \
            mv $(core_cfg_temp) $(core_cfg); \
        else \
            rm -f $(core_cfg_temp); \
        fi; \
    fi
endef
