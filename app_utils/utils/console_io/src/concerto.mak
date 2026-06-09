ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET      := app_utils_console_io
    TARGETTYPE  := library
    CSOURCES    := app_log_writer.c app_log_reader.c
    CSOURCES    += app_get.c

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),FREERTOS SAFERTOS THREADX))
        CSOURCES     += app_log_rtos.c app_cli_rtos.c

        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R52P M55 C66 C71 C7120 C7504 C7524 C7604))
            CSOURCES += app_log_printf_ticgt_rtos.c
        endif
    endif

    ifeq ($(TARGET_OS),LINUX)
        CSOURCES += app_log_linux.c
    endif

    ifeq ($(TARGET_OS),QNX)
        IDIRS    += $(PDK_QNX_PATH)/packages/
        CSOURCES += app_log_qnx.c
    endif

    include $(FINALE)

else

    include $(PRELUDE)
    TARGET     := app_utils_console_io
    TARGETTYPE := library

    CSOURCES := app_get.c

    ifeq ($(SOC), tda54)
        CSOURCES += app_log_reader.c app_log_rtos.c app_log_writer.c
        IDIRS += $(MCU_SDK_PATH)/source/device/$(SOC)
        IDIRS += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
        IDIRS += $(MCU_SDK_PATH)/source/compatibility/dpl/include
        IDIRS += $(MCU_SDK_PATH)/source/arch/include
        DEFS += FREERTOS
    endif

    include $(FINALE)

endif
