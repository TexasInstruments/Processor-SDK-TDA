# Copyright (C) 2011-2026 Texas Insruments, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


ifeq ($(BUILD_DEBUG),1)
$(info TI_TOOLS_ROOT=$(TI_TOOLS_ROOT))
$(info TIARMCGT_LLVM_ROOT=$(TIARMCGT_LLVM_ROOT))
endif

# DEP_PROJECTS does not need to be set as the dependencies are contained in the build.

TIOVX_INC  = $(TIOVX_PATH)/include $(TIOVX_PATH)/kernels/include $(TIOVX_PATH)/utils/include

SYSIDIRS := $(TIOVX_INC)
SYSIDIRS += $(IMAGING_PATH)/kernels/include
SYSIDIRS += $(VIDEO_IO_PATH)/kernels/include
SYSIDIRS += $(TIDL_PATH)/arm-tidl/tiovx_kernels/include
SYSIDIRS += $(TIDL_PATH)/arm-tidl/rt/inc
SYSIDIRS += $(VXLIB_PATH)/packages
SYSIDIRS += $(IVISION_PATH)
SYSIDIRS += $(TIADALG_PATH)/include
SYSIDIRS += $(VISION_APPS_PATH)
SYSIDIRS += $(APP_KERNELS_PATH)
SYSIDIRS += $(IMAGING_PATH)
SYSIDIRS += $(APP_UTILS_PATH)
SYSIDIRS += $(VIDEO_IO_PATH)

SYSLDIRS :=
SYSDEFS  :=

SYS_XDC_IDIRS = $(BIOS_PATH)/packages

ifeq ($(TARGET_PLATFORM),PC)
    SYSIDIRS += $(GCC_WINDOWS_ROOT)/include
    SYSLDIRS += $(GCC_WINDOWS_ROOT)/lib
else
    ifeq ($(TARGET_FAMILY),ARM)
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
            ifeq ($(TARGET_OS),SYSBIOS)
                SYSIDIRS += $(GCC_SYSBIOS_ARM_ROOT)/aarch64-elf/libc/usr/include/
                SYSLDIRS += $(GCC_SYSBIOS_ARM_ROOT)/aarch64-elf/libc/usr/lib/
            else ifeq ($(TARGET_OS),QNX)
                SYSIDIRS += $(PDK_QNX_PATH)/packages
                SYSIDIRS += $(PDK_QNX_PATH)/packages/ti/osal
                SYSIDIRS += $(PDK_QNX_PATH)/packages/ti/drv
                SYSIDIRS += $(GCC_QNX_ARM_ROOT)/../usr/include
                SYSLDIRS += $(GCC_QNX_ARM_ROOT)/../usr/lib
                SYSDEFS  += QNX_OS
                SYSDEFS  += BUILD_MPU1_0
                SYSDEFS  += $(TARGET_PLATFORM)
            else
                SYSIDIRS += $(LINUX_FS_PATH)/usr/include
                SYSLDIRS += $(LINUX_FS_PATH)/usr/lib
            endif
            INSTALL_LIB := /usr/lib
            INSTALL_BIN := /usr/bin
            INSTALL_INC := /usr/include
        else
            SYSIDIRS += $(TIARMCGT_LLVM_ROOT)/include/c
            SYSLDIRS += $(TIARMCGT_LLVM_ROOT)/lib
        endif
    else ifeq ($(TARGET_FAMILY),DSP)
        ifeq ($(TARGET_CPU),C66)
            SYSIDIRS += $(CGT6X_ROOT)/include
            SYSLDIRS += $(CGT6X_ROOT)/lib
            SYSDEFS  += C6X_FAMILY
        else
            SYSDEFS  += C7X_FAMILY
            SYSIDIRS += $(CGT7X_ROOT)/include
            SYSLDIRS += $(CGT7X_ROOT)/lib
        endif
    else ifeq ($(TARGET_FAMILY),EVE)
        SYSIDIRS += $(ARP32CGT_ROOT)/include
        SYSLDIRS += $(ARP32CGT_ROOT)/lib
    endif

    ifeq ($(TARGET_OS), $(filter $(TARGET_OS), FREERTOS SAFERTOS THREADX))
        ifeq ($(RTOS_SDK),pdk)
            SYSIDIRS += $(PDK_PATH)/packages
            SYSIDIRS += $(PDK_PATH)/packages/ti/osal
            SYSIDIRS += $(PDK_PATH)/packages/ti/drv
        else ifeq ($(RTOS_SDK),mcu_plus_sdk)
            SYSIDIRS += $(MCU_PLUS_SDK_PATH)/source
            SYSIDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers
            SYSIDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
        else
            SYSIDIRS += $(MCU_SDK_PATH)/source
            SYSIDIRS += $(MCU_SDK_PATH)/source/drivers
            SYSIDIRS += $(MCU_SDK_PATH)/source/compatibility/dpl/include
            ifeq ($(TARGET_CPU),M55)
                SYSIDIRS += $(MCU_SDK_PATH)/source/compiler/m55-ti-arm-clang
            else ifeq ($(TARGET_CPU),C7604)
                SYSIDIRS += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
            else
                SYSIDIRS += $(MCU_SDK_PATH)/source/compiler/r52-ti-arm-clang
            endif
            SYSIDIRS += $(MCU_SDK_PATH)/source/arch/include
            SYSIDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
            SYSIDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/utils_cfg
            endif
    endif

    ifeq ($(TARGET_CPU),C66)
        SYSIDIRS += $(MATHLIB_PATH)/packages
    endif
endif

ifeq ($(RTOS_SDK),mcu_plus_sdk)
    SYSDEFS  += MCU_PLUS_SDK
else ifeq ($(RTOS_SDK),pdk)
    SYSDEFS  += PDK
else
    SYSDEFS  += MCU_SDK
endif
