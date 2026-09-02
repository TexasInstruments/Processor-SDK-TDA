
include tools_path.mak
include build_flags.mak

# Project specific build defs (don't change across different combos):

BUILD_DEFS += $(SOC_DEF)
BUILD_DEFS += $(ECU_BUILD_DEF)

ifeq ($(SOC), am62a)
BUILD_DEFS += $(TISDK_IMAGE)
endif

DIRECTORIES := kernels utils

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
        ifeq ($(SOC_FAMILY),SOC_FAMILY_TDA5)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):M55:1:debug:TIARMCGT_LLVM
        else
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:debug:TIARMCGT_LLVM
        endif
    endif

    ifeq ($(PROFILE), $(filter $(PROFILE), release all))
        ifeq ($(BUILD_LINUX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):LINUX:$(MPU_CPU):1:release:GCC_LINUX_ARM
        endif
        ifeq ($(BUILD_QNX_MPU),yes)
            TARGET_COMBOS += $(TARGET_SOC):QNX:$(MPU_CPU):1:release:GCC_QNX_ARM
        endif
        ifeq ($(SOC_FAMILY),SOC_FAMILY_TDA5)
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):M55:1:release:TIARMCGT_LLVM
        else
            TARGET_COMBOS += $(TARGET_SOC):$(RTOS):R5F:1:release:TIARMCGT_LLVM
        endif
    endif
endif


CONCERTO_ROOT ?= $(PSDK_BUILDER_PATH)/concerto
BUILD_MULTI_PROJECT := 1
BUILD_TARGET := target.mak
BUILD_PLATFORM :=

include $(CONCERTO_ROOT)/rules.mak

# Project specific rules

.PHONY: all
all:
