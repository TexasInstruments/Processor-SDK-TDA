ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 A720))

include $(PRELUDE)

TARGET      := vx_app_viss

CSOURCES    := main.c

TARGETTYPE  := exe

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(VISION_APPS_KERNELS_IDIRS)

STATIC_LIBS += $(IMAGING_LIBS)

include $(FINALE)

endif
