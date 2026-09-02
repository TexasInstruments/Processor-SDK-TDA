ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

  include $(PRELUDE)
  TARGET      := vx_img_proc_kernels_test
  TARGETTYPE  := exe

  CSOURCES    := main.c
  CSOURCES    += app_tiovx_dl_color_convert_node_test.c
  CSOURCES    += app_tiovx_dl_pre_proc_node_test.c

  LDIRS       += $(PLATFORM_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS       += $(TIOVX_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS       += $(APP_UTILS_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS       += $(APP_KERNELS_PATH)/out/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS       += $(APP_KERNELS_PATH)/utils/include

  TIOVX_LIBS :=
  TIOVX_LIBS += vx_framework
  TIOVX_LIBS += vx_kernels_host_utils vx_kernels_target_utils
  TIOVX_LIBS += vx_platform_board_hlos
  TIOVX_LIBS += vx_kernels_openvx_core
  TIOVX_LIBS += vx_kernels_openvx_ext vx_target_kernels_openvx_ext
  TIOVX_LIBS += vx_utils

  APP_UTILS_LIBS =
  APP_UTILS_LIBS += app_utils_console_io
  APP_UTILS_LIBS += app_utils_file_io
  APP_UTILS_LIBS += app_utils_ipc
  APP_UTILS_LIBS += app_utils_timer
  APP_UTILS_LIBS += app_utils_remote_service
  APP_UTILS_LIBS += app_utils_mem

  STATIC_LIBS += app_init_hlos_common ipc_common
  STATIC_LIBS += vx_kernel_utils
  STATIC_LIBS += vx_kernels_img_proc vx_target_kernels_img_proc_arm

  STATIC_LIBS += $(TIOVX_LIBS)
  STATIC_LIBS += $(APP_UTILS_LIBS)

  SHARED_LIBS += ti_rpmsg_char

  SKIPBUILD=1

  include $(FINALE)

endif
