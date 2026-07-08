ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_tidl_front_cam

CSOURCES    := main.c
CSOURCES    += app_scaler_module.c
CSOURCES    += app_pre_proc_module.c
CSOURCES    += app_post_proc_module.c


ifeq ($(TARGET_OS),$(filter $(TARGET_OS), QNX))
CSOURCES    += omx_encode_module.c
endif

# CFLAGS += -Wno-unused-function
# CFLAGS += -Wno-unused-variable
# CFLAGS += -Wno-unused-but-set-variable
# CFLAGS += -Wno-implicit-function-declaration

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

TARGETTYPE  := exe
CSOURCES    += main_linux_arm.c

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

STATIC_LIBS += $(IMAGING_LIBS)
STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)

SHARED_LIBS += edgeai-apps-utils
SHARED_LIBS += edgeai-tiovx-kernels

IDIRS += $(VISION_APPS_KERNELS_IDIRS)
IDIRS += $(VISION_APPS_PATH)/modules/include
IDIRS += $(EDGEAI_KERNELS_PATH)/include
IDIRS += $(TIOVX_PATH)/source/include

endif
endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

LDIRS += $(PSDK_QNX_PATH)/qnx/codec/vpu/OpenMAXIL/core/nto/aarch64/$(BUILD_PROFILE_QNX_SO)/
LDIRS += $(PSDK_QNX_PATH)/qnx/codec/vpu/OpenMAXIL/utility/nto/aarch64/$(BUILD_PROFILE_QNX_SO)/

IDIRS += ${PSDK_QNX_PATH}/qnx/codec/vpu/OpenMAXIL/khronos/openmaxil
IDIRS += ${PSDK_QNX_PATH}/qnx/codec/vpu/OpenMAXIL/core/public/khronos/openmaxil

ifeq ($(TARGET_BUILD), release)
SHARED_LIBS += omxcore_j7$(BUILD_PROFILE_QNX_SUFFIX)
SHARED_LIBS += omxil_j7_utility$(BUILD_PROFILE_QNX_SUFFIX)
endif

ifeq ($(TARGET_BUILD), debug)
CFLAGS      += -DDEBUG_MODE
SHARED_LIBS += slog2
STATIC_LIBS += omxcore_j7$(BUILD_PROFILE_QNX_SUFFIX)S
STATIC_LIBS += omxil_j7_utility$(BUILD_PROFILE_QNX_SUFFIX)S
endif

endif
endif


ifneq ($(SOC),$(filter $(SOC), j722s j784s4 j721s2 j742s2))
SKIPBUILD=1
endif

include $(FINALE)

endif
