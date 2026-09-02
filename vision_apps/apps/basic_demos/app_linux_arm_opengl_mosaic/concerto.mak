ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))
ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)

TARGET      := vx_app_arm_opengl_mosaic
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

ifeq ($(TARGET_OS),LINUX)
CFLAGS += -DEGL_NO_X11
SYS_SHARED_LIBS += gbm
endif

ifeq ($(TARGET_OS),QNX)
SYS_SHARED_LIBS += screen
endif

endif

ifeq ($(TARGET_CPU),x86_64)
include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
# Not building for PC
SKIPBUILD=1
endif

SYS_SHARED_LIBS += EGL
SYS_SHARED_LIBS += GLESv2

IDIRS += $(APP_SAMPLE_IDIRS)

STATIC_LIBS += $(APP_SAMPLE_LIBS)
STATIC_LIBS += $(APP_OPENGL_UTILS_LIBS)

include $(FINALE)

endif
endif


