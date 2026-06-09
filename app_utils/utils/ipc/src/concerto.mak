ifneq ($(TARGET_PLATFORM),PC)

    _MODULE=app_utils_ipc
    include $(PRELUDE)
    TARGET      := app_utils_ipc
    TARGETTYPE  := library

    ifeq ($(LDRA_COVERAGE_ENABLED), yes)
        include $(PSDK_PATH)/tiovx/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
    else
        DEFS    += LDRA_UNTESTABLE_CODE
    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS THREADX))
            CSOURCES := app_ipc_rtos.c
    endif

    ifeq ($(RTOS_SDK),mcu_sdk)
        IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
        IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
        IDIRS    += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/include
        IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/include
        IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
    endif

    ifeq ($(TARGET_OS),LINUX)
        CSOURCES := app_ipc_linux.c
        CSOURCES += app_ipc_linux_hw_spinlock.c
        CSOURCES += app_ipc_linux_rpmsg_char.c
    endif

    ifeq ($(TARGET_OS), QNX)
        ifeq ($(SOC), tda54)
            CSOURCES := app_ipc_qnx_tda54.c
        else
            IDIRS    += $(PDK_QNX_PATH)/packages/ti/drv/ipc/
            IDIRS    += $(PDK_QNX_PATH)/packages/
            CSOURCES := app_ipc_qnx.c
        endif
    endif

    ifeq ($(TARGET_OS),THREADX)
        ## ThreadX is currently supported on only R5
        IDIRS    += $(MCU_PLUS_SDK_PATH)/source/kernel/threadx/ports/ti_arm_gcc_clang_cortex_r5/inc
    endif

    include $(FINALE)

else # ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET      := app_utils_ipc
    TARGETTYPE  := library

    IDIRS    +=	$(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
    IDIRS    +=	$(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
    IDIRS    += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/SchM/include
    IDIRS    += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/utils_cfg
    IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/hal/RPMessage/v0/src
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/src
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/vPC/include
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/include
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/v0/src
    IDIRS    += $(MCU_SDK_PATH)/source/drivers/RPMessage/vPC/include
    IDIRS    += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
    IDIRS    += $(MCU_SDK_PATH)/source/compatibility/dpl/include
    IDIRS    += $(MCU_SDK_PATH)/source/arch/include
    IDIRS    += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw

    CSOURCES += app_ipc_pc.c

    ifneq ($(SOC),tda54)
        SKIPBUILD=1
    endif

    include $(FINALE)

endif # ifneq ($(TARGET_PLATFORM),PC)

ifeq ($(TARGET_OS), $(filter $(TARGET_OS), SYSBIOS FREERTOS SAFERTOS THREADX))

    # app_rtos_linux library
    _MODULE=app_rtos_linux
    include $(PRELUDE)

    TARGET      := app_rtos_linux
    TARGETTYPE  := library
    CSOURCES    := app_ipc_rtos_linux.c

    include $(FINALE)

    # app_rtos_qnx library
    _MODULE=app_rtos_qnx
    include $(PRELUDE)

    TARGET      := app_rtos_qnx
    TARGETTYPE  := library

    CSOURCES    := app_ipc_rtos_qnx.c

    include $(FINALE)

endif
