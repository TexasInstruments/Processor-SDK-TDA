ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 A720))

include $(PRELUDE)
TARGET      := vx_kernels_tvm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/tvm/include
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(TIOVX_PATH)/kernels/include
IDIRS       += $(TIDL_RT_PATH)/inc

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
endif

include $(FINALE)

endif
