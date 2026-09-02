
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 A15 A720 M4 C66 EVE R5F C71 C7120 C7504 C7524 C7604))
  ifeq ($(BUILD_PTK), yes)

    include $(PRELUDE)
    TARGET      := vx_kernels_common
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)
    IDIRS       += $(HOST_ROOT)/kernels/include
    IDIRS       += $(PTK_PATH)/include

    ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
      CFLAGS += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
      DEFS += CORE_DSP CORE_C6XX
    endif

    ifeq ($(TARGET_CPU),C66)
      DEFS += CORE_DSP CORE_C6XX
    endif

    include $(FINALE)

  endif
endif
