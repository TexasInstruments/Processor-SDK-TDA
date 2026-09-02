
ifeq ($(BUILD_DEBUG),1)
$(info TI_TOOLS_ROOT=$(TI_TOOLS_ROOT))
$(info TIARMCGT_LLVM_ROOT=$(TIARMCGT_LLVM_ROOT))
endif

# DEP_PROJECTS does not need to be set as the dependencies are contained in the build.

SYSIDIRS := $(TIOVX_PATH)/include
SYSIDIRS += $(TIOVX_PATH)/kernels/include
SYSIDIRS += $(HOST_ROOT)/kernels/include

SYSLDIRS :=

SYSDEFS  :=

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721E J721S2 J784S4 J742S2 AM62A J722S TDA54))
    SYSDEFS +=
    ifeq ($(TARGET_FAMILY),ARM)
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
            ifeq ($(TARGET_OS),QNX)
                ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
                    SYSIDIRS += $(PDK_QNX_PATH)/packages
                    SYSIDIRS += $(PDK_QNX_PATH)/packages/ti/osal
                    SYSIDIRS += $(PDK_QNX_PATH)/packages/ti/drv
                else ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A720))
                    SYSIDIRS += $(PSDK_QNX_BUILD_PATH)/src/common/lld_source/source
                    SYSIDIRS += $(PSDK_QNX_BUILD_PATH)/src/common/lld_source/source/kernel/dpl
                endif
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
    endif
    ifeq ($(TARGET_OS), $(filter $(TARGET_OS), FREERTOS SAFERTOS THREADX))
        ifeq ($(RTOS_SDK),pdk)
            SYSIDIRS += $(PDK_PATH)/packages
            SYSIDIRS += $(PDK_PATH)/packages/ti/osal
            SYSIDIRS += $(PDK_PATH)/packages/ti/drv
        else ifeq ($(RTOS_SDK),mcu_sdk)
            SYSIDIRS += $(MCU_SDK_PATH)/source
            SYSIDIRS += $(MCU_SDK_PATH)/source/drivers
            SYSIDIRS += $(MCU_SDK_PATH)/source/compatibility/dpl/include
            SYSIDIRS += $(MCU_SDK_PATH)/source/device/$(SOC)
            SYSIDIRS += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
            SYSIDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
            SYSIDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
            SYSIDIRS += $(MCU_SDK_PATH)/source/arch/include
            ifeq ($(TARGET_CPU),M55)
                SYSIDIRS += $(MCU_SDK_PATH)/source/compiler/m55-ti-arm-clang
            else ifeq ($(TARGET_CPU),C7604)
                SYSIDIRS += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
            else
                SYSIDIRS += $(MCU_SDK_PATH)/source/compiler/r52-ti-arm-clang
            endif
        else ifeq ($(RTOS_SDK),mcu_plus_sdk)
            SYSIDIRS += $(MCU_PLUS_SDK_PATH)/source
            SYSIDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers
            SYSIDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
        endif
    endif
endif

ifeq ($(RTOS_SDK),mcu_plus_sdk)
    SYSDEFS  += MCU_PLUS_SDK
else ifeq ($(RTOS_SDK),pdk)
    SYSDEFS  += PDK
else ifeq ($(RTOS_SDK),mcu_sdk)
    SYSDEFS  += MCU_SDK
endif