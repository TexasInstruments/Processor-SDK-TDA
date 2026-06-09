ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F M55))
    ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)
        ifeq ($(BUILD_DISPLAY),yes)

        include $(PRELUDE)
        TARGET      := vx_target_kernels_display
        TARGETTYPE  := library
        CSOURCES    := $(call all-c-files)
        IDIRS       += $(HOST_ROOT)/kernels/include
        IDIRS       += $(HOST_ROOT)/kernels/video_io/include
        IDIRS       += $(VXLIB_PATH)/packages
        ifeq ($(RTOS_SDK), mcu_plus_sdk)
            IDIRS       += $(MCU_PLUS_SDK_PATH)/source
            IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
        else ifeq ($(RTOS_SDK), mcu_sdk)
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers/Dss/v0
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers/Dss/v0/soc
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers/Dss/v0/soc/$(SOC)
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers/Dss/v0/include
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers/Dss/v0/hw_include/V4
            IDIRS		+= $(MCU_SDK_PATH)/source/compatibility/dpl/include
            IDIRS		+= $(MCU_SDK_PATH)/source/drivers/Fvid2/v0/include
            IDIRS		+= $(MCU_SDK_PATH)/source/device/$(SOC)/include/hw
            IDIRS		+= $(MCU_SDK_PATH)/source/arch/include
        else
            IDIRS       += $(PDK_PATH)/packages
            IDIRS       += $(PDK_PATH)/packages/ti/drv
            IDIRS       += $(PDK_PATH)/packages/ti/drv/dss
        endif

        IDIRS       += $(APP_UTILS_PATH)

        ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
        DEFS += VLAB_VIDEO_IO
        endif

        include $(FINALE)

        endif
    endif
endif
