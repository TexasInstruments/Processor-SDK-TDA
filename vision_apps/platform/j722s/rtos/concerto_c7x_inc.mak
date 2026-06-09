ifeq ($(TARGET_CPU),C7524)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common
IDIRS+=$(VISION_APPS_PATH)/kernels/img_proc/include
IDIRS+=$(VISION_APPS_PATH)/kernels/fileio/include
IDIRS+=$(VISION_APPS_PATH)/kernels/srv/include
IDIRS+=$(VISION_APPS_PATH)/kernels/park_assist/include
IDIRS+=$(PTK_PATH)/include
IDIRS+=$(VISION_APPS_PATH)/kernels/stereo/include
IDIRS+=$(IMAGING_PATH)/kernels/include
IDIRS+=$(TIADALG_PATH)/include

LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/$(RTOS_TYPE)/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/dmautils/lib/

LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(IMAGING_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(PTK_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(VXLIB_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/NO_OS/$(TARGET_BUILD)
LDIRS += $(VISION_APPS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(MMALIB_PATH)/lib/$(C7X_VERSION)/$(TARGET_BUILD)
LDIRS += $(TIDL_PATH)/ti_dl/lib/$(TARGET_PLATFORM)/dsp/algo/$(TARGET_BUILD)
LDIRS += $(TIADALG_PATH)/lib/$(TARGET_CPU)/$(TARGET_BUILD)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
LDIRS += $(TIDL_PATH)/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
else
LDIRS += $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
endif

STATIC_LIBS += vx_app_c7x_target_kernel
STATIC_LIBS += vx_target_kernels_stereo
STATIC_LIBS += vx_kernels_common
#STATIC_LIBS += vx_target_kernels_srv_c66
STATIC_LIBS += vx_target_kernels_img_proc_c66

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
TIOVX_LIBS += vx_target_kernels_tidl
TIOVX_LIBS += vx_target_kernels_tvm
TIOVX_LIBS += vx_target_kernels_tvm_dynload
TIOVX_LIBS += vx_target_kernels_ivision_common
TIOVX_LIBS += vx_framework vx_platform_board_rtos vx_kernels_target_utils
TIOVX_LIBS += vx_target_kernels_tutorial
TIOVX_LIBS += vx_target_kernels_openvx_core
TIOVX_LIBS += vx_target_kernels_dsp
TIOVX_LIBS += vx_target_kernels_j7_arm
TIOVX_LIBS += vx_target_kernels_source_sink

SYS_STATIC_LIBS += $(TIOVX_LIBS)

ADDITIONAL_STATIC_LIBS += vxlib_C7524.lib

ADDITIONAL_STATIC_LIBS += board.j722s.c75x.ti-c7000.${TARGET_BUILD}.lib

ifeq ($(RTOS),FREERTOS)
	ADDITIONAL_STATIC_LIBS += freertos.j722s.c75x.ti-c7000.${TARGET_BUILD}.lib
endif

ifeq ($(RTOS),SAFERTOS)
	DEFS += OS_$(RTOS)
	ADDITIONAL_STATIC_LIBS += safertos.j722s.c75x.ti-c7000.${TARGET_BUILD}.lib
endif
ADDITIONAL_STATIC_LIBS += libc.a

endif
