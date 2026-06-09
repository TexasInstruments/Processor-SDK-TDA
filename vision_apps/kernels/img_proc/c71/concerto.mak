ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C71 C7120 C7524))

    include $(PRELUDE)
    TARGET      := vx_target_kernels_img_proc_c71
    TARGETTYPE  := library

    CSOURCES    := vx_kernels_img_proc_target.c
    CSOURCES    += vx_sfm_target.c

    IDIRS       += $(VISION_APPS_PATH)/kernels/img_proc/include
    IDIRS       += $(VISION_APPS_PATH)/kernels/img_proc/host
    IDIRS       += $(TIADALG_PATH)/include
    IDIRS       += $(IVISION_PATH)
    IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
    IDIRS       += $(VXLIB_PATH)/packages

    ifeq ($(RTOS_SDK), mcu_plus_sdk)
        IDIRS       += $(MCU_PLUS_SDK_PATH)/source
        IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
    else
        IDIRS       += $(PDK_PATH)/packages
        IDIRS       += $(PDK_PATH)/packages/ti/drv
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120))
        DEFS += CORE_DSP
    endif

    ifeq ($(TARGET_CPU), x86_64)
        DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION __aarch64__
        ifeq ($(SOC),$(filter $(SOC), am62a j722s))
        SKIPBUILD=1
        endif
    endif

    ifeq ($(SOC), tda54)
        SKIPBUILD=1
    endif

    include $(FINALE)

endif
