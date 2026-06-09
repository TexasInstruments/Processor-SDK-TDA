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

IDIRS       += $(VISION_APPS_KERNELS_IDIRS)

STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)
STATIC_LIBS += $(IMAGING_LIBS)
STATIC_LIBS += vx_kernels_img_proc
STATIC_LIBS += vx_kernels_fileio
STATIC_LIBS += vx_target_kernels_fileio

endif
endif

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(VISION_APPS_PATH)/kernels/img_proc/include
IDIRS += $(VISION_APPS_PATH)/kernels/fileio/include
IDIRS += $(VISION_APPS_PATH)/modules/include
IDIRS += $(TIOVX_PATH)/source/include


STATIC_LIBS += $(TIADALG_LIBS)
STATIC_LIBS += vx_app_modules

IDIRS       += $(EDGEAI_IDIRS)
SHARED_LIBS += edgeai-apps-utils
SHARED_LIBS += edgeai-tiovx-kernels

ifneq ($(SOC),$(filter $(SOC), j721s2 j784s4))
SKIPBUILD=1
endif

include $(FINALE)

endif
