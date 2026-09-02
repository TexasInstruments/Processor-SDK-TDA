ifeq ($(SOC), tda54)
  ifeq ($(TARGET_CPU), x86_64)

    include $(PRELUDE)

    TARGET      := app_utils_init_vdk_stub
    TARGETTYPE  := library

    IDIRS       := $(PLATFORM_PATH)
    IDIRS       += $(PLATFORM_PATH)/pc/include
    IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
    IDIRS       += $(TIOVX_PATH)/source/include
    IDIRS       += $(TIOVX_PATH)/source/platform/pc
    IDIRS       += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
    IDIRS       += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support
    IDIRS       += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
    IDIRS       += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/Hal_Cfg
    IDIRS       += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/SchM/include
    IDIRS       += $(MCU_SDK_PATH)/source/compiler/hostemu-gcc-linux
    IDIRS       += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/include
    IDIRS       += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
    IDIRS       += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/src
    IDIRS       += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/vPC/include
    IDIRS       += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/include
    IDIRS       += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/src
    IDIRS       += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
    IDIRS       += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
    IDIRS       += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
    IDIRS       += $(MCU_SDK_PATH)/source/compatibility/dpl/include
    IDIRS       += $(MCU_SDK_PATH)/source/arch/include

    CSOURCES   := ../app_init_vdk.c

    $(shell touch $(PLATFORM_PATH)/pc/src/app_init_vdk.c)

    DEFS += VDK_STUB

    include $(FINALE)

  endif

endif
