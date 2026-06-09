ifeq ($(TARGET_CPU),C7604)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common
IDIRS+=$(VISION_APPS_PATH)/kernels/img_proc/include
IDIRS+=$(VISION_APPS_PATH)/kernels/fileio/include
IDIRS+=$(VISION_APPS_PATH)/kernels/srv/include
IDIRS+=$(VISION_APPS_PATH)/kernels/park_assist/include
IDIRS+=$(VISION_APPS_PATH)/kernels/stereo/include
IDIRS+=$(IMAGING_PATH)/kernels/include
IDIRS+=$(IMAGING_PATH)/sensor_drv/include

ifeq ($(TARGET_BUILD),debug)
    MCU_SDK_BUILD_TYPE := Debug
else
    MCU_SDK_BUILD_TYPE := Release
endif
LDIRS += $(MCU_SDK_PATH)/build/$(SOC)/lib/$(MCU_SDK_BUILD_TYPE)
LDIRS += $(DMA_UTILS_PATH)/lib

LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(IMAGING_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(VXLIB_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/NO_OS/$(TARGET_BUILD)
LDIRS += $(VISION_APPS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(MMALIB_PATH)/lib/$(C7X_VERSION)/release
LDIRS += $(TIDL_PATH)/ti_dl/lib/$(TARGET_PLATFORM)/dsp/algo/$(TARGET_BUILD)
LDIRS += $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(TIDL_PATH)/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)


# STATIC_LIBS += vx_app_c7x_target_kernel
# STATIC_LIBS += vx_target_kernels_stereo
# STATIC_LIBS += vx_kernels_common
# STATIC_LIBS += vx_target_kernels_srv_c66
# STATIC_LIBS += vx_target_kernels_img_proc_c66

APP_UTILS_LIBS =
APP_UTILS_LIBS += app_utils_mem
APP_UTILS_LIBS += app_utils_rtos
APP_UTILS_LIBS += app_utils_console_io
APP_UTILS_LIBS += app_utils_timer
APP_UTILS_LIBS += app_utils_file_io
APP_UTILS_LIBS += app_utils_ipc
APP_UTILS_LIBS += app_utils_remote_service
APP_UTILS_LIBS += app_utils_misc
APP_UTILS_LIBS += app_utils_ipc_test
APP_UTILS_LIBS += app_utils_udma
APP_UTILS_LIBS += app_utils_perf_stats
# APP_UTILS_LIBS += app_utils_sciclient

SYS_STATIC_LIBS += $(APP_UTILS_LIBS)

TIOVX_LIBS =
TIOVX_LIBS += vx_framework vx_platform_board_rtos vx_kernels_target_utils
TIOVX_LIBS += vx_target_kernels_source_sink
TIOVX_LIBS += vx_target_kernels_tutorial
TIOVX_LIBS += vx_target_kernels_openvx_core
TIOVX_LIBS += vx_target_kernels_tidl
TIOVX_LIBS += vx_target_kernels_dsp
# TIOVX_LIBS += vx_target_kernels_tvm
# TIOVX_LIBS += vx_target_kernels_tvm_dynload
TIOVX_LIBS += vx_target_kernels_ivision_common
# TIOVX_LIBS += vx_target_kernels_j7_arm

TIDL_LIBS =
TIDL_LIBS += common_C7604
TIDL_LIBS += mmalib_C7604
TIDL_LIBS += mmalib_cn_C7604
TIDL_LIBS += tidl_algo
TIDL_LIBS += tidl_priv
TIDL_LIBS += tidl_kernels
TIDL_LIBS += tidl_ref
TIDL_LIBS += tidl_custom

SYS_STATIC_LIBS += $(TIOVX_LIBS)
SYS_STATIC_LIBS += $(TIDL_LIBS)

ADDITIONAL_STATIC_LIBS += vxlib_C7604.lib

ifeq ($(RTOS),FREERTOS)
    ADDITIONAL_STATIC_LIBS += libarch-ti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libtda54_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libhal-ti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libfreertos-ti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libdpl_freertos-ti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libdrivers-ti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libutils_freertos-ti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += libti_sdk_cfg_default_c76_ti-c7000.a
    ADDITIONAL_STATIC_LIBS += dmautils.tda54.dsp.ti-c7000.${TARGET_BUILD}.lib
endif

ADDITIONAL_STATIC_LIBS += libc.a

endif
