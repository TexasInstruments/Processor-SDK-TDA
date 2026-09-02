ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720 R5F M55))

  include $(PRELUDE)
  TARGET      := vx_kernels_img_proc
  TARGETTYPE  := library
  CSOURCES    := tivx_img_proc_node_api.c
  CSOURCES    += vx_kernels_img_proc_host.c

  ifeq ($(SOC), $(filter $(SOC), am62a j721e j721s2 j784s4 j742s2 j722s tda54))
    CSOURCES    += tivx_dl_pre_proc_armv8_host.c
    CSOURCES    += tivx_dl_color_convert_armv8_host.c
    CSOURCES    += tivx_dl_post_proc_host.c
    CSOURCES    += tivx_img_add.c
    ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j784s4 j742s2 j722s tda54))
      CSOURCES    += tivx_img_mosaic_host.c
      CSOURCES    += tivx_dof_plane_seperation_host.c
      CSOURCES    += tivx_img_preprocessing_host.c
      CSOURCES    += tivx_oc_pre_proc_host.c
      CSOURCES    += tivx_oc_post_proc_host.c
      CSOURCES    += tivx_od_postprocessing_host.c
      CSOURCES    += tivx_pixel_visualization_host.c
      CSOURCES    += tivx_pose_visualization_host.c
      CSOURCES    += tivx_visual_localization_host.c
      CSOURCES    += tivx_draw_keypoint_detections_host.c
      CSOURCES    += tivx_draw_box_detections_host.c
      CSOURCES    += tivx_img_hist_host.c
      CSOURCES    += tivx_sfm_host.c
      CSOURCES    += tivx_dl_pre_proc_host.c
      CSOURCES    += tivx_dl_color_blend_host.c
      CSOURCES    += tivx_dl_draw_box_host.c
      CSOURCES    += tivx_dl_color_convert_host.c
      ifeq ($(SOC), $(filter $(SOC), j784s4 tda54))
        CSOURCES    += tivx_dl_bev_pre_proc_armv8_host.c
        CSOURCES    += tivx_dl_bev_post_proc_host.c
        CSOURCES    += tivx_dl_bev_cam_post_proc_host.c
      endif
    endif
  endif

  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS       += $(TIDL_PATH)/arm-tidl/rt/inc

  include $(FINALE)

endif
