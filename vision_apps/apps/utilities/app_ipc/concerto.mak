ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)

TARGETTYPE  := exe
TARGET      := vx_app_arm_ipc
CSOURCES    := $(call all-c-files)

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

include $(FINALE)

endif
endif

