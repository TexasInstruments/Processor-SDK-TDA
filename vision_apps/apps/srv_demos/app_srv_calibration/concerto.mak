ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

include $(PRELUDE)

TARGET      := vx_app_srv_calibration
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(VISION_APPS_APPLIBS_IDIRS)
IDIRS += $(APP_SRV_IDIRS)
IDIRS += $(APP_KERNELS_IDIRS)

STATIC_LIBS += $(IMAGING_LIBS)
STATIC_LIBS += $(APP_OPENGL_UTILS_LIBS)
STATIC_LIBS += $(APP_SRV_LIBS)
STATIC_LIBS += $(APP_KERNELS_LIBS)

ifeq ($(TARGET_OS),QNX)
SYS_SHARED_LIBS += screen
endif

ifeq ($(TARGET_OS),LINUX)
SYS_SHARED_LIBS += gbm
endif

SYS_SHARED_LIBS += EGL
SYS_SHARED_LIBS += GLESv2


include $(FINALE)

endif
