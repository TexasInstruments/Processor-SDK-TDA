ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 x86_64 A720))

    include $(PRELUDE)
    TARGET      := vx_target_kernels_img_proc_a72
    TARGETTYPE  := library

    CSOURCES    := vx_kernels_img_proc_target.c
    CSOURCES    += vx_img_hist_target.c

    ifeq ($(SOC), $(filter $(SOC), j784s4 tda54))
        CSOURCES    += vx_dl_bev_pre_proc_armv8_target.c
        CSOURCES    += vx_dl_bev_post_proc_target.c
        CSOURCES    += vx_dl_bev_cam_post_proc_target.c
        CSOURCES    += vx_nv12_drawing_utils.c
        CSOURCES    += vx_nv12_font_utils.c
    endif

    IDIRS       += $(VISION_APPS_PATH)/kernels/img_proc/include
    IDIRS       += $(VISION_APPS_PATH)/kernels/img_proc/host
    IDIRS       += $(TIOVX_PATH)/kernels/ivision/include

    include $(FINALE)

endif
