ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_multi_cam
TARGETTYPE  := exe

CSOURCES    := main.c

ifeq ($(TARGET_CPU),x86_64)
include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
CSOURCES    += main_x86.c
# Not building for PC
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
CSOURCES    += main_linux_arm.c
endif
endif

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(APP_KERNELS_IDIRS)
IDIRS += $(VISION_APPS_MODULES_IDIRS)

STATIC_LIBS += $(IMAGING_LIBS)
STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

include $(FINALE)

endif
