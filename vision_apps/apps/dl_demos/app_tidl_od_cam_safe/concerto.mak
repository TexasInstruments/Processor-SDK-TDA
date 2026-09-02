ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72))
include $(PRELUDE)

TARGET      := vx_app_tidl_od_cam_safe

CSOURCES    := main.c
CSOURCES    += app_dd_module_queued.c
CSOURCES    += app_pre_proc_module_queued.c


ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-function
endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72))

TARGETTYPE  := exe

CSOURCES    += main_linux_arm.c

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

IDIRS       += $(APP_KERNELS_IDIRS)

STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)
STATIC_LIBS += $(IMAGING_LIBS)

endif
endif

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(VISION_APPS_PATH)/modules/include
IDIRS += $(TIOVX_PATH)/source/include

STATIC_LIBS += $(TIADALG_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

ifneq ($(SOC),$(filter $(SOC), j721s2 j784s4))
SKIPBUILD=1
endif

include $(FINALE)

endif
