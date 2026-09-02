ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 A720))
  ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

    include $(PRELUDE)
    TARGET      := vx_target_kernels_srv_gpu
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)
    CFLAGS      += -DGL_ES
    IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include
    IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/gpu
    IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/gpu/3dsrv
    IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/host
    IDIRS       += $(VXLIB_PATH)/packages
    IDIRS       += $(APP_KERNELS_PATH)/utils/include

    ifeq ($(TARGET_OS), LINUX)
      IDIRS       += $(LINUX_FS_PATH)/usr/include
      IDIRS       += $(LINUX_FS_PATH)/usr/include/drm
      CFLAGS      += -DEGL_NO_X11
    endif

    ifeq ($(TARGET_OS), QNX)
      IDIRS       += $(QNX_TARGET)/usr/include
    endif

    ifeq ($(SOC),$(filter $(SOC), am62a))
      SKIPBUILD=1
    endif

    include $(FINALE)

  endif
endif

ifeq ($(TARGET_CPU),x86_64)

  include $(PRELUDE)
  TARGET      := vx_target_kernels_srv_gpu
  TARGETTYPE  := library
  CSOURCES    := $(call all-c-files)
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/gpu
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/target/gpu/3dsrv
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/host
  IDIRS       += $(VXLIB_PATH)/packages
  IDIRS       += $(APP_KERNELS_PATH)/utils/include
  CFLAGS      += -DGL_ES -DSTANDALONE

  DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION

  ifeq ($(SOC),$(filter $(SOC), am62a))
    SKIPBUILD=1
  endif

  include $(FINALE)

endif
