ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_sfm

CSOURCES    := main.c
CSOURCES    += app_pyramid_module.c
CSOURCES    += app_dof_module.c
CSOURCES    += app_sfm_module.c
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

IDIRS       += $(APP_KERNELS_IDIRS)
IDIRS       += $(VISION_APPS_MODULES_IDIRS)

STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)

endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

TARGETTYPE  := exe

CSOURCES    += main_linux_arm.c

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

IDIRS       += $(APP_KERNELS_IDIRS)
IDIRS       += $(VISION_APPS_MODULES_IDIRS)

STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

endif
endif

ifeq ($(SOC),j722s)
SKIPBUILD=1
endif

include $(FINALE)

endif
