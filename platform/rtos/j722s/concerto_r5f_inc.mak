ifeq ($(TARGET_CPU),R5F)

  CFLAGS  += -mthumb
  LDFLAGS += --ram_model --priority

  IDIRS+=$(PLATFORM_PATH)
  IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/fileio/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/srv/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/park_assist/include
  IDIRS+=$(APP_KERNELS_PATH)/kernels/stereo/include
  IDIRS+=$(PTK_PATH)/include
  IDIRS+=$(IMAGING_PATH)/kernels/include
  IDIRS+=$(IMAGING_PATH)/sensor_drv/include

  LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/$(RTOS_TYPE)/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/

  LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(APP_KERNELS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(IMAGING_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
  LDIRS += $(VIDEO_IO_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

  STATIC_LIBS += vx_target_kernels_img_proc_mcu
  STATIC_LIBS += app_utils_mpu

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
  APP_UTILS_LIBS += app_utils_misc
  APP_UTILS_LIBS += app_utils_perf_stats
  APP_UTILS_LIBS += app_utils_hwa

  SYS_STATIC_LIBS += $(APP_UTILS_LIBS)

  TIOVX_LIBS =
  TIOVX_LIBS += vx_conformance_engine vx_conformance_tests vx_conformance_tests_testmodule
  TIOVX_LIBS += vx_tiovx_tests vx_tiovx_internal_tests vx_tutorial vx_utils
  TIOVX_LIBS += vx_framework vx_vxu vx_platform_board_rtos vx_kernels_target_utils
  TIOVX_LIBS += vx_kernels_test_kernels_tests vx_kernels_test_kernels
  TIOVX_LIBS += vx_target_kernels_openvx_ext
  TIOVX_LIBS += vx_target_kernels_source_sink
  TIOVX_LIBS += vx_kernels_hwa_tests vx_kernels_hwa
  TIOVX_LIBS += vx_target_kernels_vpac_viss vx_target_kernels_vpac_msc vx_target_kernels_vpac_ldc vx_target_kernels_vpac_fc vx_target_kernels_vpac_aewb
  TIOVX_LIBS += vx_target_kernels_dmpac_dof vx_target_kernels_dmpac_sde
  TIOVX_LIBS += vx_target_kernels_capture
  TIOVX_LIBS += vx_target_kernels_csitx
  TIOVX_LIBS += vx_target_kernels_j7_arm
  TIOVX_LIBS += vx_target_kernels_display
  ifeq ($(LDRA_COVERAGE_ENABLED_IMAGING), yes)
    TIOVX_LIBS += imaging_coverage
  endif

  IMAGING_LIBS  = ti_imaging_awbalg
  IMAGING_LIBS += ti_imaging_dcc
  IMAGING_LIBS += vx_kernels_imaging
  IMAGING_LIBS += vx_target_kernels_imaging_aewb
  IMAGING_LIBS += ti_imaging_aealg
  IMAGING_LIBS += ti_imaging_sensordrv
  IMAGING_LIBS += ti_imaging_ittsrvr
  IMAGING_LIBS += app_utils_sensors
  IMAGING_LIBS += app_utils_iss

  SYS_STATIC_LIBS += $(TIOVX_LIBS)
  SYS_STATIC_LIBS += $(IMAGING_LIBS)


  ADDITIONAL_STATIC_LIBS += board.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib

  ifeq ($(RTOS),FREERTOS)
    ADDITIONAL_STATIC_LIBS += freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  endif

  ifeq ($(RTOS),SAFERTOS)
    DEFS += OS_$(RTOS)
    ADDITIONAL_STATIC_LIBS += safertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  endif

endif
