ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C71 C7120 C7524 C7504))

  include $(PRELUDE)
  TARGET      := vx_target_kernels_add_img
  TARGETTYPE  := library

  CSOURCES    := $(call all-c-files)

  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
  IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/host

  ifeq ($(TARGET_CPU), x86_64)
    CFLAGS += --std=c++14 -D_HOST_EMULATION -pedantic -fPIC -w -c -g
    CFLAGS += -Wno-sign-compare
  endif

  include $(FINALE)

endif
