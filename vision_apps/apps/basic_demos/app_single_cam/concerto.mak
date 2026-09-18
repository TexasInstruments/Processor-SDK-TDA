ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A720 A72 A53))

include $(PRELUDE)

CSOURCES    := app_single_cam_common.c
TARGET      := vx_app_single_cam
TARGETTYPE  := exe

ifeq ($(TARGET_CPU),x86_64)
CSOURCES    += main_x86.c
include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
STATIC_LIBS += $(IMAGING_LIBS)
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A720 A72 A53))
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
CSOURCES    += app_single_cam_main.c
CSOURCES    += main_linux_arm.c
include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
STATIC_LIBS += $(IMAGING_LIBS)

# AM62A Linux: Link drm_wrapper for DRM/KMS display with zero-copy
ifeq ($(TARGET_PLATFORM)$(TARGET_OS), AM62ALINUX)
STATIC_LIBS += app_utils_drm_wrapper
SYS_SHARED_LIBS += drm
IDIRS += $(LINUX_FS_PATH)/usr/include/libdrm/
IDIRS += $(LINUX_FS_PATH)/usr/include/drm/
endif

endif
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A720 A72 A53))
ifeq ($(TARGET_OS),SYSBIOS)

TARGETTYPE  := library

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

endif
endif

IDIRS += $(IMAGING_IDIRS)

include $(FINALE)

endif
