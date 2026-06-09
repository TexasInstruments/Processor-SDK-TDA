
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F ))
    ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)
        ifeq ($(BUILD_CSITX),yes)

            include $(PRELUDE)
            TARGET      := vx_target_kernels_csitx
            TARGETTYPE  := library
            CSOURCES    := $(call all-c-files)
            IDIRS       += $(HOST_ROOT)/kernels/include
            IDIRS       += $(HOST_ROOT)/kernels/video_io/include
            IDIRS       += $(VXLIB_PATH)/packages
            ifeq ($(RTOS_SDK), mcu_plus_sdk)
                IDIRS       += $(MCU_PLUS_SDK_PATH)/source
                IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
            else
                IDIRS       += $(PDK_PATH)/packages
                IDIRS       += $(PDK_PATH)/packages/ti/drv
                IDIRS       += $(PDK_PATH)/packages/ti/drv/csitx
                IDIRS       += $(PDK_PATH)/packages/ti/drv/fvid2
            endif
            
            # JIRA: ADASVISION-7147 - Enable capture kernels in video_io once csirx driver is enabled
            ifeq ($(SOC), tda54)
                SKIPBUILD=1
            endif

            include $(FINALE)

        endif
    endif
endif
