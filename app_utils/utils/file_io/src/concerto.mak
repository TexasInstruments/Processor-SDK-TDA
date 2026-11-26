ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET      := app_utils_file_io
    TARGETTYPE  := library
    CSOURCES    := app_fileio_common.c

    IDIRS += $(APP_UTILS_PATH)/utils/mem/include
    IDIRS += $(APP_UTILS_PATH)/utils/file_io/include

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS THREADX))
        CSOURCES += app_fileio_rtos.c app_fileio_client_rtos.c
    else
        CSOURCES += app_fileio_server.c app_fileio_client_hlos.c
    endif

    ifeq ($(TARGET_OS),LINUX)
        CSOURCES += app_fileio_linux.c
    endif

    ifeq ($(TARGET_OS),QNX)
        CSOURCES += app_fileio_qnx.c
    endif

    ifeq ($(CODE_COVERAGE_ENABLED_FOR_TIDL), yes)
        SYSDEFS  += CODE_COVERAGE_ENABLED_FOR_TIDL
    endif

    ifeq ($(LDRA_COVERAGE_ENABLED), yes)
        SYSDEFS  += LDRA_COVERAGE_ENABLED
    endif

    include $(FINALE)

else

    include $(PRELUDE)
    TARGET      := app_utils_file_io
    TARGETTYPE  := library

    IDIRS += $(APP_UTILS_PATH)/utils/file_io/include
    CSOURCES := app_fileio_common.c
    ifeq ($(VDK),yes)
        IDIRS += $(MCU_SDK_PATH)/source/compiler/hostemu-gcc-linux
        IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/v0/include
        IDIRS += $(MCU_SDK_PATH)/source/hal/Ipc_Notify/vPC/include
        IDIRS += $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
        IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/Hal_Cfg
        IDIRS += $(MCU_SDK_PATH)/ti_sdk_config/$(SOC)/default/device_support/include
        CSOURCES += app_fileio_rtos.c app_fileio_client_rtos.c
    else
        CSOURCES += app_fileio_client_hlos.c
    endif

    include $(FINALE)

endif
