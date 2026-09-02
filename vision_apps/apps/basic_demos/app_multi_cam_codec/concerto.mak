ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_multi_cam_codec
TARGETTYPE  := exe

CSOURCES    := main.c multi_cam_codec_ldc_module.c multi_cam_codec_scaler_module.c multi_cam_codec_img_mosaic_module.c app_common.c

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
CFLAGS += -Wno-stringop-overflow
endif
endif

IDIRS += $(IMAGING_IDIRS)
IDIRS += $(APP_KERNELS_IDIRS)
IDIRS += $(VISION_APPS_MODULES_IDIRS)

STATIC_LIBS += $(IMAGING_LIBS)
STATIC_LIBS += $(APP_KERNELS_LIBS)
STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

STATIC_LIBS += app_utils_codec_wrapper

ifeq ($(TARGET_OS), LINUX)
STATIC_LIBS += app_utils_gst_wrapper

SHARED_LIBS += gstreamer-1.0
SHARED_LIBS += gstapp-1.0
SHARED_LIBS += gstbase-1.0
SHARED_LIBS += gobject-2.0
SHARED_LIBS += glib-2.0

CFLAGS += -Wno-restrict
endif

ifeq ($(TARGET_OS), QNX)
STATIC_LIBS += app_utils_omax_wrapper

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J784S4 J742S2 J721S2 J722S AM62A))
LDIRS += $(PSDK_QNX_PATH)/qnx/codec/vpu/OpenMAXIL/core/nto/aarch64/$(BUILD_PROFILE_QNX_SO)/
LDIRS += $(PSDK_QNX_PATH)/qnx/codec/vpu/OpenMAXIL/utility/nto/aarch64/$(BUILD_PROFILE_QNX_SO)/

CFLAGS      += -DCODEC_USE_HIGHMEM
endif
ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721E))
LDIRS += $(PSDK_QNX_PATH)/qnx/codec/img/qnx/OpenMAXIL/core/nto/aarch64/$(BUILD_PROFILE_QNX_SO)/
LDIRS += $(PSDK_QNX_PATH)/qnx/codec/img/qnx/OpenMAXIL/utility/nto/aarch64/$(BUILD_PROFILE_QNX_SO)/
endif

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

include $(FINALE)

endif
endif
