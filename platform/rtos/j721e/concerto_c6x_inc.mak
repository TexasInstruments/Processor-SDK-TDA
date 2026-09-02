ifeq ($(TARGET_CPU),C66)

  IDIRS+=$(PLATFORM_PATH)
  IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/fileio/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/srv/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/park_assist/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/stereo/include
  IDIRS+=$(PTK_PATH)/include
  IDIRS+=$(IMAGING_PATH)/kernels/include

  ifeq ($(RTOS),SYSBIOS)
    LDIRS += $(PDK_PATH)/packages/ti/osal/lib/tirtos/$(SOC)/c66/$(TARGET_BUILD)/
  endif
  ifeq ($(RTOS),FREERTOS)
    LDIRS += $(PDK_PATH)/packages/ti/osal/lib/freertos/$(SOC)/c66/$(TARGET_BUILD)/
  endif
  ifeq ($(RTOS),SAFERTOS)
    LDIRS += $(PDK_PATH)/packages/ti/osal/lib/safertos/$(SOC)/c66/$(TARGET_BUILD)/
  endif
  LDIRS += $(PDK_PATH)/packages/ti/csl/lib/$(SOC)/c66/$(TARGET_BUILD)/
  LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(APP_KERNELS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(IMAGING_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(PTK_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(VXLIB_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/NO_OS/$(TARGET_BUILD)
  LDIRS += $(TIADALG_PATH)/lib/$(TARGET_CPU)/$(TARGET_BUILD)
  LDIRS += $(MATHLIB_PATH)/packages/ti/mathlib/lib

  SYS_STATIC_LIBS += vx_target_kernels_img_proc_c6x
  SYS_STATIC_LIBS += vx_kernels_common
  SYS_STATIC_LIBS += vx_target_kernels_stereo
  SYS_STATIC_LIBS += vx_target_kernels_srv_c6x

  APP_UTILS_LIBS =
  APP_UTILS_LIBS += app_utils_mem
  APP_UTILS_LIBS += app_utils_rtos
  APP_UTILS_LIBS += app_utils_console_io
  APP_UTILS_LIBS += app_utils_timer
  APP_UTILS_LIBS += app_utils_file_io
  APP_UTILS_LIBS += app_utils_ipc
  APP_UTILS_LIBS += app_utils_ipc_test
  APP_UTILS_LIBS += app_utils_remote_service
  APP_UTILS_LIBS += app_utils_udma
  APP_UTILS_LIBS += app_utils_sciclient
  APP_UTILS_LIBS += app_utils_misc
  APP_UTILS_LIBS += app_utils_perf_stats

  SYS_STATIC_LIBS += $(APP_UTILS_LIBS)

  PTK_LIBS =
  PTK_LIBS += ptk_algos
  PTK_LIBS += ptk_base

  SYS_STATIC_LIBS += $(PTK_LIBS)

  TIOVX_LIBS =
  TIOVX_LIBS += vx_framework vx_platform_board_rtos vx_kernels_target_utils
  TIOVX_LIBS += vx_target_kernels_openvx_core
  TIOVX_LIBS += vx_target_kernels_tutorial
  TIOVX_LIBS += vx_target_kernels_dsp
  TIOVX_LIBS += vx_target_kernels_source_sink
  TIOVX_LIBS += vx_target_kernels_ivision_common
  TIOVX_LIBS += vx_target_kernels_j7_arm

  SYS_STATIC_LIBS += $(TIOVX_LIBS)

  ADDITIONAL_STATIC_LIBS += vxlib_C66.lib

  ADDITIONAL_STATIC_LIBS += ti.osal.ae66
  ADDITIONAL_STATIC_LIBS += ipc.ae66
  ADDITIONAL_STATIC_LIBS += sciclient.ae66
  ADDITIONAL_STATIC_LIBS += udma.ae66
  ADDITIONAL_STATIC_LIBS += mathlib.ae66

  ifeq ($(RTOS),FREERTOS)
    ADDITIONAL_STATIC_LIBS += ti.kernel.freertos.ae66
  endif
  ifeq ($(RTOS),SAFERTOS)
    ADDITIONAL_STATIC_LIBS += ti.kernel.safertos.ae66
  endif

  ADDITIONAL_STATIC_LIBS += ti.csl.ae66

  ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
    ADDITIONAL_STATIC_LIBS += ti.csl.intc.ae66
  endif

  ADDITIONAL_STATIC_LIBS += libtiadalg_fisheye_transformation.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_image_preprocessing.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_dof_plane_seperation.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_select_top_feature.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_sparse_upsampling.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_visual_localization.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_solve_pnp.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_image_color_blending.a
  ADDITIONAL_STATIC_LIBS += libtiadalg_image_recursive_nms.a
  ADDITIONAL_STATIC_LIBS += libc.a

  SYS_STATIC_LIBS += rts6600_elf

endif
