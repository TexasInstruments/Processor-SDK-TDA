ifeq ($(TARGET_CPU),R52P)

    IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common
    IDIRS+=$(VISION_APPS_PATH)/kernels/img_proc/include
    IDIRS+=$(VISION_APPS_PATH)/kernels/fileio/include
    IDIRS+=$(VISION_APPS_PATH)/kernels/srv/include
    IDIRS+=$(VISION_APPS_PATH)/kernels/park_assist/include
    IDIRS+=$(VISION_APPS_PATH)/kernels/stereo/include
    IDIRS+=$(IMAGING_PATH)/kernels/include
    IDIRS+=$(IMAGING_PATH)/sensor_drv/include

    # MCU_SDK uses capitalized Debug/Release
    ifeq ($(TARGET_BUILD),debug)
        MCU_SDK_BUILD_TYPE := Debug
    else
        MCU_SDK_BUILD_TYPE := Release
    endif

    LDIRS += $(MCU_SDK_PATH)/build/$(SOC)/lib/$(MCU_SDK_BUILD_TYPE)
    LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

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
    # APP_UTILS_LIBS += app_utils_sciclient

    SYS_STATIC_LIBS += $(APP_UTILS_LIBS)

    TIOVX_LIBS =
    TIOVX_LIBS += vx_framework vx_platform_board_rtos
    TIOVX_LIBS += vx_target_kernels_source_sink vx_kernels_target_utils
    TIOVX_LIBS += vx_target_kernels_openvx_ext

    SYS_STATIC_LIBS += $(TIOVX_LIBS)

    ifeq ($(RTOS),FREERTOS)
        ADDITIONAL_STATIC_LIBS += libfreertos-ti_sdk_cfg_default_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libdpl_freertos-ti_sdk_cfg_default_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libtda54_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libhal-ti_sdk_cfg_default_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libti_sdk_cfg_default_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libarch-ti_sdk_cfg_default_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libutils_freertos-ti_sdk_cfg_default_r52_ti-arm-clang.a
        ADDITIONAL_STATIC_LIBS += libdrivers-ti_sdk_cfg_default_r52_ti-arm-clang.a
    endif

endif
