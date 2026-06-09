ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_vpac_aewb
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests
IDIRS       += $(HOST_ROOT)/source/include

ifeq ($(RTOS_SDK), mcu_plus_sdk)
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa
else
  IDIRS       += $(PDK_PATH)/packages
  IDIRS       += $(PDK_PATH)/packages/ti/drv
  IDIRS       += $(PDK_PATH)/packages/ti/drv/vhwa
endif
IDIRS       += $(APP_UTILS_PATH)/

IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(HOST_ROOT)/kernels/hwa/include
IDIRS       += $(HOST_ROOT)/algos/dcc/include
IDIRS       += $(HOST_ROOT)/algos/awb/include
IDIRS       += $(VXLIB_PATH)/packages

IDIRS       += $(APP_UTILS_PATH)/utils/remote_service/include
IDIRS       += $(APP_UTILS_PATH)/utils/ipc/include

IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(TIOVX_PATH)/conformance_tests
IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(VISION_APPS_PATH)/applibs
IDIRS       += $(VISION_APPS_PATH)/kernels/srv/include
IDIRS       += $(VISION_APPS_PATH)/kernels/srv/c66
IDIRS       += $(VISION_APPS_PATH)
IDIRS       += $(VISION_APPS_PATH)/kernels/srv/gpu/3dsrv
IDIRS       += $(IMAGING_PATH)/kernels/include
IDIRS       += $(IMAGING_PATH)/sensor_drv/include

CFLAGS      += -DHAVE_VERSION_INC
ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-result
endif

ifeq ($(RTOS_SDK),mcu_plus_sdk)
CFLAGS+= -DMCU_PLUS_SDK
endif

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

ifeq ($(SOC), am62a)
ifeq ($(TISDK_IMAGE), edgeai)
ifneq ($(TARGET_OS), QNX)
SKIPBUILD=1
endif
endif
endif

include $(FINALE)

endif
