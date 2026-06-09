ifeq ($(TARGET_PLATFORM),TDA54)
    ifeq ($(TARGET_OS), $(filter $(TARGET_OS), FREERTOS SAFERTOS))

    include $(PRELUDE)
    TARGET      := app_rtos
    TARGETTYPE  := library

    CSOURCES    += ipc_trace.c

    IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common

    include $(FINALE)

    endif
endif
