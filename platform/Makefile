include tools_path.mak

# Edit below file to change default build options
include build_flags.mak

# Project specific build defs (don't change across different combos):
BUILD_DEFS :=
BUILD_DEFS += $(SOC_DEF)
BUILD_DEFS += $(ECU_BUILD_DEF)
BUILD_DEFS += $(foreach CORE,$(ENABLED_IPC_CORES),$(CORE))

ifeq ($(LDRA_COVERAGE_ENABLED_IMAGING), yes)
BUILD_DEFS += LDRA_COVERAGE
endif

DIRECTORIES :=
DIRECTORIES += hlos
DIRECTORIES += pc
DIRECTORIES += utils
DIRECTORIES += rtos/$(SOC)

TARGET_COMBOS :=

ifeq ($(BUILD_EMULATION_MODE),yes)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        TARGET_COMBOS += PC:LINUX:x86_64:1:debug:GCC_LINUX
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        TARGET_COMBOS += PC:LINUX:x86_64:1:release:GCC_LINUX
    endif
endif

ifeq ($(BUILD_TARGET_MODE),yes)
    ifeq ($(PROFILE), $(filter $(PROFILE), debug all))
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:debug:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:debug:GCC_QNX_ARM
        endif
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:debug:CGT7X
        ifeq ($(SOC_FAMILY),SOC_FAMILY_TDA5)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):M55:1:debug:TIARMCGT_LLVM
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R52P:1:debug:TIARMCGT_LLVM
        else
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:debug:TIARMCGT_LLVM
        endif
        ifeq ($(TARGET_SOC),J721E)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:1:debug:CGT6X
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:release:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:release:GCC_QNX_ARM
        endif
        TARGET_COMBOS += $(TARGET_SOC):$(RTOS):$(C7X_TARGET):1:release:CGT7X
        ifeq ($(SOC_FAMILY),SOC_FAMILY_TDA5)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):M55:1:release:TIARMCGT_LLVM
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R52P:1:release:TIARMCGT_LLVM
        else
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:release:TIARMCGT_LLVM
        endif
        ifeq ($(TARGET_SOC),J721E)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):C66:1:release:CGT6X
        endif
    endif
endif

CONCERTO_ROOT ?= $(PSDK_BUILDER_PATH)/concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak

# Project specific rules

.PHONY: hlos pc rtos all check_ipc_cores

all:

check_ipc_cores:
	$(call CHECK_IPC_CORES,$(PLATFORM_PATH),$(ENABLED_IPC_CORES))

hlos:
	$(MAKE) -C $(PLATFORM_PATH)  app_init_hlos_common
	TARGET_CPU=$(MPU_CPU) $(MAKE) -C $(PLATFORM_PATH)  ipc_common

pc:
	$(MAKE) -C $(PLATFORM_PATH) app_init_pc_common
ifeq ($(VDK),yes)
	$(MAKE) -C $(PLATFORM_PATH) app_utils_init_vdk
	$(MAKE) -C $(PLATFORM_PATH) app_utils_init_vdk_stub
endif

rtos: all

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
            touch $(PLATFORM_PATH)/rtos/$(SOC)/common/src/app_init.c; \
            touch $(PLATFORM_PATH)/hlos/src/app_init_linux.c; \
            touch $(PLATFORM_PATH)/hlos/src/app_init_qnx.c; \
            mv $(core_cfg_temp) $(core_cfg); \
        else \
            rm -f $(core_cfg_temp); \
        fi; \
    fi
endef
