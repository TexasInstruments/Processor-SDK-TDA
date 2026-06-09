ifeq ($(BUILD_APP_RTOS_LINUX),yes)
    ifeq ($(BUILD_CPU_C7x_2),yes)
        ifeq ($(TARGET_CPU),C7604)

        # OS_ID must be set before include $(PRELUDE)
        OS_ID=linux

        _MODULE=$(OS_ID)
        include $(PRELUDE)

        TARGET      := vx_app_rtos_linux_c7x_2
        TARGETTYPE  := exe
        CSOURCES    := $(call all-c-files)

        include $($(_MODULE)_SDIR)/concerto_c7x_2_inc.mak

        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common_linux
        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

        STATIC_LIBS += app_rtos_linux

        include $(FINALE)

        endif
    endif
endif

ifeq ($(BUILD_APP_RTOS_QNX),yes)
    ifeq ($(BUILD_CPU_C7x_2),yes)
        ifeq ($(TARGET_CPU),C7604)

        # OS_ID must be set before include $(PRELUDE)
        OS_ID=qnx

        _MODULE=$(OS_ID)
        include $(PRELUDE)

        TARGET      := vx_app_rtos_qnx_c7x_2
        TARGETTYPE  := exe
        CSOURCES    := $(call all-c-files)

        include $($(_MODULE)_SDIR)/concerto_c7x_2_inc.mak

        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common_qnx
        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

        STATIC_LIBS += app_rtos_qnx

        include $(FINALE)

        endif
    endif
endif
