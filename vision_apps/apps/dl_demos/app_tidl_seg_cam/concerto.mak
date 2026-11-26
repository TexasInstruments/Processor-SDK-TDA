ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_tidl_seg_cam
TARGETTYPE  := exe

CPPSOURCES  := main.cpp
CPPSOURCES  += config/app_seg_cam_cfg_params.cpp

CFLAGS += -Wno-parentheses -Wno-unused-function -Wno-sign-compare

ifeq ($(TARGET_CPU),x86_64)
include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
# Not building for PC
SKIPBUILD=1
endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
ifeq ($(TARGET_OS),QNX)
SHARED_LIBS += c++
endif
endif
endif

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(APP_KERNELS_IDIRS)
IDIRS += $(VISION_APPS_MODULES_IDIRS)
STATIC_LIBS += $(IMAGING_LIBS)
STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(APP_KERNELS_UTILS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)

ifeq ($(SOC),j722s)
SKIPBUILD=1
endif

include $(FINALE)

endif
