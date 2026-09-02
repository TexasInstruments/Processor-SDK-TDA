ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

  include $(PRELUDE)
  TARGET      := vx_kernel_utils
  TARGETTYPE  := library
  CSOURCES    := vx_arm_neon_utils.c
  CSOURCES    += vx_dl_color_convert_armv8_utils.c
  CSOURCES    += vx_dl_pre_proc_armv8_utils.c
  CSOURCES    += vx_dl_scaler_armv8_utils.c
  CSOURCES    += vx_nv12_drawing_utils.c
  CSOURCES    += vx_nv12_font_utils.c
  CSOURCES    += vx_perf_stats_utils.c
  CSOURCES    += vx_tiovx_kernels_utils.c
  CSOURCES    += vx_overlay_perf_stats_utils.c

  IDIRS       += $(HOST_ROOT)/utils/include

  include $(FINALE)

endif
