ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66 C7120 C7524))

    include $(PRELUDE)
    TARGET      := vx_target_kernels_img_proc_c66
    TARGETTYPE  := library

    CSOURCES    := vx_kernels_img_proc_target.c
    CSOURCES    += vx_dof_plane_seperation_target.c
    CSOURCES    += vx_oc_pre_proc_target.c
    CSOURCES    += vx_oc_post_proc_target.c
    CSOURCES    += vx_od_postprocessing_target.c
    CSOURCES    += vx_pixel_visualization_target.c
    CSOURCES    += vx_pose_visualization_target.c
    CSOURCES    += vx_visual_localization_target.c
    CSOURCES    += vx_draw_keypoint_detections_target.c
    CSOURCES    += vx_draw_box_detections_target.c
    CSOURCES    += vx_dl_color_blend_target.c
    CSOURCES    += vx_dl_draw_box_target.c
    CSOURCES    += vx_dma_transfers.c

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66 ))
        CSOURCES    += vx_dl_color_convert_target.c
        CSOURCES    += vx_dl_pre_proc_target.c
        CSOURCES    += vx_image_preprocessing_target.c

    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7120))
        CSOURCES    += vx_dl_pre_proc_target_no_dma.c
        CSOURCES    += vx_image_preprocessing_target_no_dma.c
    endif

    IDIRS       += $(VISION_APPS_PATH)/kernels/common/target
    IDIRS       += $(VISION_APPS_PATH)/kernels/img_proc/include
    IDIRS       += $(VISION_APPS_PATH)/kernels/img_proc/host
    IDIRS       += $(TIADALG_PATH)/include
    IDIRS       += $(IVISION_PATH)
    IDIRS       += $(TIDL_PATH)/arm-tidl/rt/inc
    IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
    IDIRS       += $(TIOVX_PATH)/kernels/openvx-core/include
    IDIRS       += $(VXLIB_PATH)/packages

    ifeq ($(TARGET_CPU),C66)
        DEFS += CORE_DSP
    endif

    ifeq ($(BUILD_BAM),yes)
        DEFS += BUILD_BAM
    endif

    ifeq ($(TARGET_CPU), x86_64)
        DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
        ifeq ($(SOC),am62a)
            SKIPBUILD=1
        endif
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7120))
        DEFS += C6X_MIGRATION _TMS320C6600 __C7120__
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7524))
        DEFS += C6X_MIGRATION _TMS320C6600 __C7524__
    endif

    ifeq ($(SOC),tda54)
        ifneq ($(TARGET_CPU), x86_64)
            SKIPBUILD=1
        endif
    endif

    include $(FINALE)

endif
