ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))

include $(PRELUDE)

TARGETTYPE  := exe
TARGET      := vx_app_arm_mem

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
CSOURCES    := main_mem.c
include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
endif

ifeq ($(TARGET_CPU), x86_64)
CPPSOURCES  := main_mem_x86.cpp
CPPFLAGS    := --std=c++11
include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
endif

include $(FINALE)

endif
endif
