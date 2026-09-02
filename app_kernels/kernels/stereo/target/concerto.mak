ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66 C71 C7120 C7524 C7604 A72 A53 A720))

  include $(PRELUDE)
  TARGET      := vx_target_kernels_stereo
  TARGETTYPE  := library
  CSOURCES    := $(call all-c-files)
  IDIRS       += $(APP_KERNELS_PATH)/kernels/stereo/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/stereo/host
  IDIRS       += $(APP_KERNELS_PATH)/kernels/common/target
  IDIRS       += $(HOST_ROOT)/kernels/include
  IDIRS       += $(VXLIB_PATH)/packages
  IDIRS       += $(PTK_PATH)/include

  ifeq ($(TARGET_CPU),C66)
    DEFS += CORE_DSP
  endif

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
    DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
  endif

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7120))
    DEFS += C6X_MIGRATION _TMS320C6600 __C7120__
  endif

  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C7604))
    DEFS += C6X_MIGRATION _TMS320C6600 __C7604__
  endif

  ifeq ($(SOC),$(filter $(SOC), am62a))
    SKIPBUILD=1
  endif

  include $(FINALE)
endif
