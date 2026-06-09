
ifneq ($(TARGET_PLATFORM), PC)
include $(PRELUDE)

CSOURCES    := tidl_rt_linux_arm.c

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721E j721e))
TARGET_OVX_PATH ?= J721E
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721S2 j721s2))
TARGET_OVX_PATH ?= J721S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J784S4 j784s4))
TARGET_OVX_PATH ?= J784S4
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J742S2 j742s2))
TARGET_OVX_PATH ?= J742S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
TARGET_OVX_PATH ?= AM62A
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
TARGET_OVX_PATH ?= J722S
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
TARGET_OVX_PATH ?= TDA54
endif

LDIRS += $(TIOVX_PATH)/lib/$(TARGET_OVX_PATH)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(VISION_APPS_PATH)/out/$(TARGET_OVX_PATH)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(LINUX_FS_PATH)/usr/lib
LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_OVX_PATH)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/$(TARGET_OVX_PATH)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

TIOVX_LIBS  =
TIOVX_LIBS += vx_framework
#TIOVX_LIBS += vx_platform_psdk_j7
TIOVX_LIBS += vx_kernels_host_utils
TIOVX_LIBS += vx_kernels_tidl
TIOVX_LIBS += vx_nested_kernels_tidl
TIOVX_LIBS += vx_kernels_tvm

TIOVX_LIBS += vx_kernels_openvx_core
TIOVX_LIBS += vx_utils

VISION_APPS_UTILS_LIBS += app_utils_console_io
VISION_APPS_UTILS_LIBS += app_utils_file_io
VISION_APPS_UTILS_LIBS += app_utils_ipc
VISION_APPS_UTILS_LIBS += app_rtos_linux_mpu1_common
VISION_APPS_UTILS_LIBS += app_utils_remote_service
VISION_APPS_UTILS_LIBS += app_utils_mem
VISION_APPS_UTILS_LIBS += app_utils_perf_stats
VISION_APPS_UTILS_LIBS += app_utils_init

# STATIC_LIBS += $(TIOVX_LIBS)
# STATIC_LIBS += $(VISION_APPS_UTILS_LIBS)

SHARED_LIBS += ti_rpmsg_char
SHARED_LIBS += tivision_apps
SHARED_LIBS += rt

include $($(_MODULE)_SDIR)/../concerto_common.mak

include $(FINALE)

endif
