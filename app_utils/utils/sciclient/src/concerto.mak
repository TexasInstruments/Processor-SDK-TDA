ifneq ($(TARGET_PLATFORM),PC)
    ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS THREADX))

    include $(PRELUDE)
    TARGET      := app_utils_sciclient
    TARGETTYPE  := library

    CSOURCES    := app_sciclient.c
    # JIRA: ADASVISION-7145 - Enable once it is enabled in mcu_sdk
    ifeq ($(SOC), tda54)
        SKIPBUILD=1
    endif

    include $(FINALE)

    endif
endif
