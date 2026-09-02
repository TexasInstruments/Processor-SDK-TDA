ifeq ($(TARGET_CPU),M55)

  IDIRS+=$(PLATFORM_PATH)
  IDIRS+=$(PLATFORM_PATH)/platform/$(SOC)/rtos/common
  IDIRS+=$(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/fileio/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/srv/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/park_assist/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/stereo/include
  IDIRS+=$(IMAGING_PATH)/kernels/include
  IDIRS+=$(IMAGING_PATH)/sensor_drv/include
  IDIRS+=$(MCU_SDK_PATH)/source/drivers/Udma/v0/soc/$(SOC)
  IDIRS+=$(MCU_SDK_PATH)/source/hal/Bcdma/v0/include
  IDIRS+=$(MCU_SDK_PATH)/source/hal/Dru/v0/include
  IDIRS+=$(MCU_SDK_PATH)/source/hal/Psilcfg/v0/include


  # MCU_SDK uses capitalized Debug/Release
  ifeq ($(TARGET_BUILD),debug)
    MCU_SDK_BUILD_TYPE := Debug
  else
    MCU_SDK_BUILD_TYPE := Release
  endif

  LDIRS += $(MCU_SDK_PATH)/build/$(SOC)/lib/$(MCU_SDK_BUILD_TYPE)
  LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(APP_KERNELS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(IMAGING_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(VIDEO_IO_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

  #STATIC_LIBS += vx_target_kernels_img_proc_mcu

  APP_UTILS_LIBS =
  APP_UTILS_LIBS += app_utils_mem
  APP_UTILS_LIBS += app_utils_rtos
  APP_UTILS_LIBS += app_utils_console_io
  APP_UTILS_LIBS += app_utils_timer
  APP_UTILS_LIBS += app_utils_file_io
  APP_UTILS_LIBS += app_utils_ipc
  APP_UTILS_LIBS += app_utils_remote_service
  APP_UTILS_LIBS += app_utils_misc
  APP_UTILS_LIBS += app_utils_udma
  APP_UTILS_LIBS += app_utils_perf_stats
  APP_UTILS_LIBS += app_utils_ipc_test
  # APP_UTILS_LIBS += app_utils_sciclient

  SYS_STATIC_LIBS += $(APP_UTILS_LIBS)

  TIOVX_LIBS =
  TIOVX_LIBS += vx_framework vx_platform_board_rtos vx_kernels_target_utils
  TIOVX_LIBS += vx_target_kernels_source_sink
  TIOVX_LIBS += vx_target_kernels_display
  TIOVX_LIBS += vx_target_kernels_openvx_ext
  # TIOVX_LIBS += vx_kernels_hwa_tests vx_kernels_hwa
  # TIOVX_LIBS += vx_target_kernels_vpac_viss vx_target_kernels_vpac_msc vx_target_kernels_vpac_ldc
  # TIOVX_LIBS += vx_target_kernels_dmpac_dof vx_target_kernels_dmpac_sde
  # TIOVX_LIBS += vx_target_kernels_capture
  # TIOVX_LIBS += vx_target_kernels_csitx
  # TIOVX_LIBS += vx_target_kernels_j7_arm
  # TIOVX_LIBS += vx_target_kernels_display_m2m

  SYS_STATIC_LIBS += $(TIOVX_LIBS)

  # IMAGING_LIBS  = ti_imaging_awbalg
  # IMAGING_LIBS += ti_imaging_dcc
  # IMAGING_LIBS += vx_kernels_imaging
  # IMAGING_LIBS += vx_target_kernels_imaging_aewb
  # IMAGING_LIBS += ti_imaging_aealg
  # IMAGING_LIBS += ti_imaging_sensordrv
  # IMAGING_LIBS += ti_imaging_ittsrvr
  # IMAGING_LIBS += app_utils_sensors
  # IMAGING_LIBS += app_utils_iss

  # SYS_STATIC_LIBS += $(IMAGING_LIBS)

  ifeq ($(RTOS),FREERTOS)
    ADDITIONAL_STATIC_LIBS += libfreertos-ti_sdk_cfg_default_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libdpl_freertos-ti_sdk_cfg_default_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libtda54_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libhal-ti_sdk_cfg_default_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libti_sdk_cfg_default_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libarch-ti_sdk_cfg_default_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libutils_freertos-ti_sdk_cfg_default_m55_ti-arm-clang.a
    ADDITIONAL_STATIC_LIBS += libdrivers-ti_sdk_cfg_default_m55_ti-arm-clang.a
  endif

endif
