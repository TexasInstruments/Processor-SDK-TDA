ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_stereo_depth
CSOURCES    := main.c

ifeq ($(TARGET_CPU),x86_64)

TARGETTYPE  := exe
CSOURCES    += main_x86.c

include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

SYSLDIRS += /usr/lib64

ifeq ($(SOC),$(filter $(SOC), j722s))
SKIPBUILD=1
endif

endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

TARGETTYPE  := exe
CSOURCES    += main_linux_arm.c

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

endif

IDIRS += $(APP_KERNELS_IDIRS)
IDIRS += $(APP_STEREO_KERNELS_IDIRS)
IDIRS += $(PTK_IDIRS)

STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(PTK_LIBS)
STATIC_LIBS += $(APP_STEREO_LIBS)

include $(FINALE)

endif
