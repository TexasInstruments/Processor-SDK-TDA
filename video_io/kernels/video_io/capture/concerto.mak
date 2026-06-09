ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)
    ifeq ($(BUILD_CAPTURE), yes)
        ifneq ($(SOC),am62a)
            ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F))

                include $(PRELUDE)
                TARGET      := vx_target_kernels_capture
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
                    IDIRS       += $(PDK_PATH)/packages/ti/drv/csirx
                    IDIRS       += $(PDK_PATH)/packages/ti/drv/fvid2
                endif

                IDIRS       += $(TIOVX_PATH)/source/include

                # JIRA: ADASVISION-7147 - Enable capture kernels in video_io once csirx driver is enabled
                ifeq ($(SOC), tda54)
                    SKIPBUILD=1
                endif

                include $(FINALE)

            endif
        #Build target on R5F for both Linux and QNX incase of adas use case
        else ifeq ($(SOC), am62a)
            ifeq ($(TISDK_IMAGE), adas)
                ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), R5F))
                    include $(PRELUDE)
                    TARGET      := vx_target_kernels_capture
                    TARGETTYPE  := library
                    CSOURCES    := $(call all-c-files)
                    IDIRS       += $(HOST_ROOT)/kernels/include
                    IDIRS       += $(HOST_ROOT)/kernels/video_io/include
                    IDIRS       += $(VXLIB_PATH)/packages
                    IDIRS       += $(MCU_PLUS_SDK_PATH)/source
                    IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
                    IDIRS       += $(TIOVX_PATH)/source/include
                    include $(FINALE)
                endif
            #Build target on A53 for am62a qnx incase of edgeai use case
            else ifeq ($(TARGET_OS), QNX)
                include $(PRELUDE)
                TARGET      := vx_target_kernels_capture
                TARGETTYPE  := library
                CSOURCES    := $(call all-c-files)
                IDIRS       += $(HOST_ROOT)/kernels/include
                IDIRS       += $(HOST_ROOT)/kernels/video_io/include
                IDIRS       += $(VXLIB_PATH)/packages
                IDIRS       += $(PDK_QNX_PATH)/packages/ti/drv/csirx
                IDIRS       += $(PDK_QNX_PATH)/packages/ti/drv/fvid2
                IDIRS       += $(TIOVX_PATH)/source/include
                include $(FINALE)
            endif
        endif # ifneq ($(SOC),am62a)
    endif # ifeq ($(BUILD_CAPTURE), yes)
endif # ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)
