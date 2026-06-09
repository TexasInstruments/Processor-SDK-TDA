ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_tidl_vl

CSOURCES    := main.c
CSOURCES    += app_pre_proc_module.c
CSOURCES    += app_tidl_module.c
CSOURCES    += app_pose_viz_module.c
CSOURCES    += app_pose_calc_module.c

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-function
endif

ifeq ($(HOST_COMPILER),GCC)
CFLAGS += -Wno-unused-function
endif

ifeq ($(TARGET_CPU),x86_64)

TARGETTYPE  := exe

CSOURCES    += main_x86.c

include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

IDIRS       += $(VISION_APPS_KERNELS_IDIRS)
IDIRS       += $(VISION_APPS_MODULES_IDIRS)

STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)

endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

TARGETTYPE  := exe

CSOURCES    += main_linux_arm.c

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

IDIRS       += $(VISION_APPS_KERNELS_IDIRS)
IDIRS       += $(VISION_APPS_MODULES_IDIRS)

STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

endif
endif

IDIRS       += $(EDGEAI_IDIRS)
SHARED_LIBS += edgeai-apps-utils
SHARED_LIBS += edgeai-tiovx-kernels

ifeq ($(SOC),j722s)
SKIPBUILD=1
endif

include $(FINALE)

endif
