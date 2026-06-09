ifeq ($(TARGET_OS), $(filter $(TARGET_OS), QNX FREERTOS SAFERTOS THREADX))

    include $(PRELUDE)

    TARGET      := app_utils_rtos
    TARGETTYPE  := library

    ifeq ($(LDRA_COVERAGE_ENABLED), yes)
        include $(PSDK_PATH)/tiovx/tiovx_dev/internal_docs/coverage_files/concerto_inc.mak
    else
        DEFS += LDRA_UNTESTABLE_CODE
    endif

    ifeq ($(TARGET_OS), $(filter $(TARGET_OS), FREERTOS SAFERTOS THREADX))
        ifeq ($(RTOS_SDK),pdk)
            CSOURCES := app_rtos_pdk.c
        else
            CSOURCES := app_rtos_mcu.c
        endif
    else
        ifeq ($(TARGET_PLATFORM)$(TARGET_OS), TDA54QNX)
            CSOURCES := app_rtos_mcu.c
        else
            CSOURCES := app_rtos_pdk.c
        endif
    endif

    include $(FINALE)

endif
