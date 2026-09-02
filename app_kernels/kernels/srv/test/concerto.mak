ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

  include $(PRELUDE)
  TARGET      := vx_kernels_srv_tests
  TARGETTYPE  := library

  CSOURCES    := test_point_detect.c test_pose_estimation.c  test_generate_3dbowl.c test_generate_gpulut.c

  ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
    ifeq ($(TARGET_OS),LINUX)
      CFLAGS      += -DGL_ES -DEGL_NO_X11
      CSOURCES    += test_gpu_srv.c
      IDIRS       += $(LINUX_FS_PATH)/usr/include
      IDIRS       += $(LINUX_FS_PATH)/usr/include/drm
    endif
  endif

  ifeq ($(TARGET_CPU),x86_64)
    CFLAGS      += -DGL_ES -DSTANDALONE
    CSOURCES    += test_gpu_srv.c
  endif

  IDIRS       += $(TIOVX_PATH)/conformance_tests
  IDIRS       += $(TIOVX_PATH)/conformance_tests/test_tiovx
  IDIRS       += $(TIOVX_PATH)/source/include
  IDIRS       += $(TIOVX_PATH)/utils/include
  IDIRS       += $(APP_KERNELS_PATH)
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/host
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/dsp
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/gpu/3dsrv
  IDIRS       += $(APP_KERNELS_PATH)/utils/include
  IDIRS       += $(VXLIB_PATH)/packages
  IDIRS       += $(TIOVX_PATH)/include
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

  ifeq ($(SOC),$(filter $(SOC), j722s am62a))
    SKIPBUILD=1
  endif

  include $(FINALE)
endif
