ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 C66))

  include $(PRELUDE)
  TARGET      := vx_target_kernels_srv_c6x
  TARGETTYPE  := library
  CSOURCES    := $(call all-c-files)
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/host
  IDIRS       += $(APP_KERNELS_PATH)
  IDIRS       += $(VXLIB_PATH)/packages
  DEFS += CORE_DSP

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
    DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
    IDIRS       += $(VXLIB_PATH)/packages/ti/vxlib/src/common/c6xsim
    CFLAGS += -Wno-maybe-uninitialized
  endif

  include $(FINALE)

endif

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721S2 J784S4 J742S2 J722S TDA54))

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7120 C7524 C7604))

    include $(PRELUDE)
    TARGET      := vx_target_kernels_srv_c7x
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)
    IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/include
    IDIRS       += $(APP_KERNELS_PATH)/kernels/srv/host
    IDIRS       += $(APP_KERNELS_PATH)
    IDIRS       += $(VXLIB_PATH)/packages
    DEFS        += CORE_DSP

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7120))
      DEFS += C6X_MIGRATION _TMS320C6600
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7524))
      DEFS += C6X_MIGRATION _TMS320C6600 __C7524__
    endif

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7604))
      DEFS += C6X_MIGRATION _TMS320C6600 __C7604__
    endif

    include $(FINALE)

  endif

endif
