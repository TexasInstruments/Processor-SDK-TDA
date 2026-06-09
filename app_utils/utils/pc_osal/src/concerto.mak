ifeq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET      := app_utils_pc_osal
    TARGETTYPE  := library

    CSOURCES    := $(call all-c-files)

    IDIRS += $(MCU_SDK_PATH)/source/compatibility/dpl/include
    IDIRS += $(MCU_SDK_PATH)/source/compiler/hostemu-gcc-linux
    IDIRS += $(MCU_SDK_PATH)/source/arch/include
    IDIRS += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
    IDIRS += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/src
    IDIRS += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/vPC/include
    IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
    IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/src
    IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
    IDIRS += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/Hal_Cfg
    IDIRS += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
    IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
    IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support
    IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include

    ifneq ($(SOC),tda54)
        SKIPBUILD=1
    endif

    include $(FINALE)

endif # ifeq ($(TARGET_PLATFORM),PC)
