ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)
    TARGET     := app_utils_timer
    TARGETTYPE := library

    ifeq ($(LDRA_COVERAGE_ENABLED), yes)
        include $(PSDK_PATH)/tiovx/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
    else
        DEFS   += LDRA_UNTESTABLE_CODE
    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),FREERTOS SAFERTOS THREADX))
        CSOURCES := app_timer_rtos.c
        IDIRS   += $(MCU_SDK_PATH)/source/drivers/Ipc_Notify/v0/sciclient_stub
    endif

    ifeq ($(TARGET_OS),LINUX)
        CSOURCES += app_timer_linux.c
    endif

    ifeq ($(TARGET_OS),QNX)
        IDIRS    += $(PDK_QNX_PATH)/packages/
        CSOURCES += app_timer_qnx.c
    endif

    include $(FINALE)

else # ifneq ($(TARGET_PLATFORM),PC)

    include $(PRELUDE)

    TARGET      := app_utils_timer
    TARGETTYPE  := library

    CSOURCES := app_timer_rtos.c

    IDIRS    += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
    IDIRS    += $(MCU_SDK_PATH)/source/compatibility/dpl/include
    IDIRS    += $(MCU_SDK_PATH)/source/arch/include
    IDIRS    += $(MCU_SDK_PATH)/source/device/$(SOC)

    ifneq ($(SOC),tda54)
        SKIPBUILD=1
    endif

    include $(FINALE)
endif
