ifeq ($(TARGET_CPU),C7504)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos/common
IDIRS+=$(VISION_APPS_PATH)/kernels/img_proc/include
IDIRS+=$(VISION_APPS_PATH)/kernels/fileio/include
IDIRS+=$(VISION_APPS_PATH)/kernels/srv/include
IDIRS+=$(VISION_APPS_PATH)/kernels/park_assist/include
IDIRS+=$(VISION_APPS_PATH)/kernels/stereo/include
IDIRS+=$(IMAGING_PATH)/kernels/include

ifeq ($(RTOS_SDK),pdk)
	ifeq ($(RTOS),SYSBIOS)
		LDIRS += $(PDK_PATH)/packages/ti/osal/lib/tirtos/$(SOC)/c7x/$(TARGET_BUILD)/
	endif
	ifeq ($(RTOS),FREERTOS)
		LDIRS += $(PDK_PATH)/packages/ti/osal/lib/freertos/$(SOC)/c75x/$(TARGET_BUILD)/
	endif
	LDIRS += $(PDK_PATH)/packages/ti/csl/lib/$(SOC)/c75x/$(TARGET_BUILD)/
	LDIRS += $(PDK_PATH)/packages/ti/drv/uart/lib/$(SOC)/c75x/$(TARGET_BUILD)/
	LDIRS += $(PDK_PATH)/packages/ti/drv/ipc/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
	LDIRS += $(PDK_PATH)/packages/ti/drv/mailbox/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
	LDIRS += $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
	LDIRS += $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)/c7x_1/$(TARGET_BUILD)/
else
	ifeq ($(RTOS),FREERTOS)
		LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/freertos/lib/
	endif
	LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/
	LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/
	LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/dmautils/lib/
endif
LDIRS += $(VXLIB_PATH)/lib/$(TARGET_PLATFORM)/C7504/NO_OS/$(TARGET_BUILD)
LDIRS += $(TIOVX_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(VISION_APPS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(MMALIB_PATH)/lib/$(C7X_VERSION)/$(TARGET_BUILD)
LDIRS += $(TIDL_PATH)/ti_dl/lib/$(TARGET_PLATFORM)/dsp/algo/$(TARGET_BUILD)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
LDIRS += $(TIDL_PATH)/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
else
LDIRS += $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
endif

STATIC_LIBS += vx_app_c7x_target_kernel
#STATIC_LIBS += vx_target_kernels_img_proc_c71

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

TIOVX_LIBS =
TIOVX_LIBS += vx_target_kernels_tidl
TIOVX_LIBS += vx_target_kernels_tvm
TIOVX_LIBS += vx_target_kernels_tvm_dynload
TIOVX_LIBS += vx_target_kernels_ivision_common
TIOVX_LIBS += vx_framework vx_platform_board_rtos vx_kernels_target_utils
TIOVX_LIBS += vx_target_kernels_tutorial
TIOVX_LIBS += vx_target_kernels_openvx_core
TIOVX_LIBS += vx_target_kernels_dsp
TIOVX_LIBS += vx_target_kernels_source_sink

SYS_STATIC_LIBS += $(TIOVX_LIBS)

ifeq ($(RTOS_SDK),pdk)
	ADDITIONAL_STATIC_LIBS += ti.osal.ae71
	ADDITIONAL_STATIC_LIBS += ipc.ae71
	ADDITIONAL_STATIC_LIBS += mailbox.ae71
	ADDITIONAL_STATIC_LIBS += dmautils.ae71
	ADDITIONAL_STATIC_LIBS += sciclient.ae71
	ADDITIONAL_STATIC_LIBS += ti.drv.uart.ae71

	ifeq ($(RTOS),FREERTOS)
		ADDITIONAL_STATIC_LIBS += ti.kernel.freertos.ae71
	endif
	ADDITIONAL_STATIC_LIBS += ti.csl.ae71
else
	ADDITIONAL_STATIC_LIBS += board.am62ax.c75x.ti-c7000.${TARGET_BUILD}.lib
	ADDITIONAL_STATIC_LIBS += drivers.am62ax.c75x.ti-c7000.${TARGET_BUILD}.lib
	ADDITIONAL_STATIC_LIBS += dmautils.am62ax.c75x.ti-c7000.${TARGET_BUILD}.lib

	ifeq ($(RTOS),FREERTOS)
		ADDITIONAL_STATIC_LIBS += freertos.am62ax.c75x.ti-c7000.${TARGET_BUILD}.lib
	endif
endif
ADDITIONAL_STATIC_LIBS += vxlib_C7504.lib
ADDITIONAL_STATIC_LIBS += libc.a
endif
