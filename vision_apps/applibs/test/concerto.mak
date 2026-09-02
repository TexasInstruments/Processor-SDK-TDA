
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)
TARGET      := vx_applib_tests
TARGETTYPE  := library

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
CSOURCES    := test_srv_calib_applib.c
CSOURCES    += test_srv_bowl_lut_gen_applib.c
CSOURCES    += test_srv_app.c

ifneq ($(SOC), am62a)
CSOURCES    += test_vpac_viss_aewb.c
endif

endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
CSOURCES    := test_srv_calib_applib.c
CSOURCES    += test_srv_bowl_lut_gen_applib.c
CSOURCES    += test_srv_app.c

ifneq ($(SOC), am62a)
CSOURCES    += test_vpac_viss_aewb.c
endif

CFLAGS      += -DEGL_NO_X11
ifeq ($(TARGET_OS),LINUX)
IDIRS       += $(LINUX_FS_PATH)/usr/include
IDIRS       += $(LINUX_FS_PATH)/usr/include/drm
endif

ifeq ($(TARGET_OS),QNX)
IDIRS += $(GCC_QNX_ARM_ROOT)/../usr/include
LDIRS += $(GCC_QNX_ARM_ROOT)/../usr/lib
endif

endif

IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(TIOVX_PATH)/conformance_tests
IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(VISION_APPS_PATH)/applibs
IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include
IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/dsp
IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/gpu/3dsrv
IDIRS       += $(APP_KERNELS_PATH)/utils/include
IDIRS       += $(IMAGING_PATH)/kernels/include
IDIRS       += $(IMAGING_PATH)/sensor_drv/include

CFLAGS      += -DHAVE_VERSION_INC

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_SYSBIOS_ARM GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

ifeq ($(TARGET_CPU),x86_64)
CFLAGS      += -DTARGET_X86_64
endif


include $(FINALE)
endif
