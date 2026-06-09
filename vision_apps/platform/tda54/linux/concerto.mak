ifeq ($(BUILD_CPU_MPU1),yes)
    ifeq ($(TARGET_CPU),A720)
        ifeq ($(TARGET_OS),LINUX)

        include $(PRELUDE)

        TARGET      := app_rtos_linux_mpu1_common
        TARGETTYPE  := library
        CSOURCES    := $(call all-c-files)

        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/linux
        IDIRS+=$(VISION_APPS_PATH)/platform/common
        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos
        IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common

        include $(FINALE)
        endif
    endif
endif
