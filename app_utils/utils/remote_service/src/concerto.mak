ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET      := app_utils_remote_service
    TARGETTYPE  := library

    CSOURCES := app_remote_service_test.c

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS THREADX))
        CSOURCES += app_remote_service.c
        ifeq ($(RTOS_SDK),mcu_sdk)
            IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
            IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
            IDIRS    += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/include
            IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/include
            IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
        endif
    endif
    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),QNX))
        ifeq ($(SOC), tda54)
            CSOURCES += app_remote_service_qnx_tda5.c
        else
            CSOURCES += app_remote_service_qnx.c
            IDIRS    += $(PDK_QNX_PATH)/packages/ti/drv/ipc/
        endif
    endif

    ifeq ($(TARGET_OS),LINUX)
        CSOURCES += app_remote_service_linux.c
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7120 C7524 C71))
        DEFS += ENABLE_MMA_LOAD_TEST
        IDIRS += $(MMALIB_PATH)/ti/mmalib/src
    endif

    ifeq ($(TARGET_OS),THREADX)
        ## ThreadX is currently supported on only R5
        IDIRS    += $(MCU_PLUS_SDK_PATH)/source/kernel/threadx/ports/ti_arm_gcc_clang_cortex_r5/inc
    endif

    include $(FINALE)

else # ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET      := app_utils_remote_service
    TARGETTYPE  := library

    IDIRS    += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
    IDIRS    += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support
    IDIRS    += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/src
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/vPC/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/src
    IDIRS    += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/device_support
    IDIRS    += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/SchM/include
    IDIRS    += $(MCU_SDK_PATH)/source/device/$(SOC)/ti_sdk_config/default/Hal_Cfg
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/src
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/vPC/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
    IDIRS    += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
    IDIRS    += $(MCU_SDK_PATH)/source/compatibility/dpl/include
    IDIRS    += $(MCU_SDK_PATH)/source/arch/include
    IDIRS    += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw

    CSOURCES := app_remote_service_pc.c app_remote_service_test.c

    ifneq ($(SOC),tda54)
        SKIPBUILD=1
    endif

    include $(FINALE)

endif # ifneq ($(TARGET_PLATFORM),PC)
