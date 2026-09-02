
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 x86_64 A720))

  include $(PRELUDE)
  TARGET      := vx_target_kernels_img_proc_arm
  TARGETTYPE  := library

  CSOURCES    := vx_kernels_img_proc_target.c
  CSOURCES    += vx_img_hist_target.c
  CSOURCES    += vx_dl_color_convert_armv8_target.c
  CSOURCES    += vx_dl_pre_proc_armv8_target.c
  CSOURCES    += vx_dl_post_proc_target.c

  ifeq ($(SOC), $(filter $(SOC), j784s4 tda54))
    CSOURCES    += vx_dl_bev_pre_proc_armv8_target.c
    CSOURCES    += vx_dl_bev_post_proc_target.c
    CSOURCES    += vx_dl_bev_cam_post_proc_target.c
  endif

  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/host
  IDIRS       += $(APP_KERNELS_PATH)/utils/include
  IDIRS       += $(TIOVX_PATH)/kernels/ivision/include

  include $(FINALE)

endif
