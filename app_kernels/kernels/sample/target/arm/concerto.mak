ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 A720))
  ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

    include $(PRELUDE)
    TARGET      := vx_target_kernels_sample_arm
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)
    IDIRS       += $(APP_KERNELS_PATH)/kernels/sample/include
    IDIRS       += $(APP_KERNELS_PATH)/kernels/sample/host
    IDIRS       += $(APP_KERNELS_PATH)/utils/include

    ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX))
      CFLAGS      += -DEGL_NO_X11
      IDIRS       += $(LINUX_FS_PATH)/usr/include
    endif

    ifeq ($(TARGET_OS), $(filter $(TARGET_OS), QNX))
      IDIRS       += $(QNX_TARGET)/usr/include
    endif

    include $(FINALE)

  endif
endif

ifeq ($(TARGET_CPU),x86_64)

  include $(PRELUDE)
  TARGET      := vx_target_kernels_sample_arm
  TARGETTYPE  := library
  CSOURCES    := $(call all-c-files)
  IDIRS       += $(APP_KERNELS_PATH)/utils/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/sample/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/sample/host
  IDIRS       += $(HOST_ROOT)/kernels/include
  IDIRS       += $(VXLIB_PATH)/packages

  DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION

  include $(FINALE)

endif
