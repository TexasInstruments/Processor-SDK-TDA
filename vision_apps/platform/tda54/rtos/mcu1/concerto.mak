ifeq ($(BUILD_APP_RTOS_LINUX),yes)
    ifeq ($(BUILD_CPU_MCU1),yes)
        ifeq ($(TARGET_CPU),M55)

        # OS_ID must be set before include $(PRELUDE)
        OS_ID=linux

        _MODULE=$(OS_ID)
        include $(PRELUDE)

        TARGET      := vx_app_rtos_linux_mcu1
        TARGETTYPE  := exe
        CSOURCES    := main.c

        include $($(_MODULE)_SDIR)/concerto_mcu1_inc.mak

        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common_linux

        STATIC_LIBS += app_rtos_linux

        include $(FINALE)

        endif
    endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
    ifeq ($(BUILD_CPU_MCU1),yes)
        ifeq ($(TARGET_CPU),M55)

        # OS_ID must be set before include $(PRELUDE)
        OS_ID=qnx

        _MODULE=$(OS_ID)
        include $(PRELUDE)

        TARGET      := vx_app_rtos_qnx_mcu1
        TARGETTYPE  := exe
        CSOURCES    := main.c

        include $($(_MODULE)_SDIR)/concerto_mcu1_inc.mak

        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common_qnx

        STATIC_LIBS += app_rtos_qnx

        include $(FINALE)

        endif
    endif
endif
