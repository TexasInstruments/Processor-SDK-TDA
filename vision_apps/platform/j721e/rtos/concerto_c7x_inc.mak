ifeq ($(TARGET_CPU),C71)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common
IDIRS+=$(VISION_APPS_PATH)/kernels/img_proc/include
IDIRS+=$(VISION_APPS_PATH)/kernels/fileio/include
IDIRS+=$(VISION_APPS_PATH)/kernels/srv/include
IDIRS+=$(VISION_APPS_PATH)/kernels/park_assist/include
IDIRS+=$(PTK_PATH)/include
IDIRS+=$(VISION_APPS_PATH)/kernels/stereo/include
IDIRS+=$(IMAGING_PATH)/kernels/include
IDIRS+=$(TIADALG_PATH)/include

ifeq ($(RTOS),SYSBIOS)
	LDIRS += $(PDK_PATH)/packages/ti/osal/lib/tirtos/$(SOC)/c7x/$(TARGET_BUILD)/
endif
ifeq ($(RTOS),FREERTOS)
	LDIRS += $(PDK_PATH)/packages/ti/osal/lib/freertos/$(SOC)/c7x/$(TARGET_BUILD)/
endif
ifeq ($(RTOS),SAFERTOS)
	LDIRS += $(PDK_PATH)/packages/ti/osal/lib/safertos/$(SOC)/c7x/$(TARGET_BUILD)/
endif
LDIRS += $(PDK_PATH)/packages/ti/csl/lib/$(SOC)/c7x/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/ipc/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(PTK_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
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
STATIC_LIBS += vx_target_kernels_img_proc_c71

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
TIOVX_LIBS += vx_target_kernels_dsp
TIOVX_LIBS += vx_target_kernels_tvm
TIOVX_LIBS += vx_target_kernels_tvm_dynload
TIOVX_LIBS += vx_target_kernels_ivision_common
TIOVX_LIBS += vx_framework vx_platform_board_rtos vx_kernels_target_utils
TIOVX_LIBS += vx_target_kernels_source_sink

SYS_STATIC_LIBS += $(TIOVX_LIBS)

ADDITIONAL_STATIC_LIBS += ti.osal.ae71
ADDITIONAL_STATIC_LIBS += ipc.ae71
ADDITIONAL_STATIC_LIBS += dmautils.ae71
ADDITIONAL_STATIC_LIBS += sciclient.ae71
ADDITIONAL_STATIC_LIBS += udma.ae71

ifeq ($(RTOS),FREERTOS)
	ADDITIONAL_STATIC_LIBS += ti.kernel.freertos.ae71
endif
ifeq ($(RTOS),SAFERTOS)
	ADDITIONAL_STATIC_LIBS += ti.kernel.safertos.ae71
endif
ADDITIONAL_STATIC_LIBS += ti.csl.ae71
ADDITIONAL_STATIC_LIBS += ti.csl.init.ae71

ADDITIONAL_STATIC_LIBS += libtiadalg_structure_from_motion.a
ADDITIONAL_STATIC_LIBS += libc.a

endif
