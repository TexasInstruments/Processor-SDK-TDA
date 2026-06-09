ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), A72 A53 A720))

ifeq ($(BUILD_CONFORMANCE_TEST),yes)

include $(PRELUDE)
TARGET      := vx_tiovx_tvm_tests
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/conformance_tests
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_RT_PATH)/inc
IDIRS       += $(TIOVX_PATH)/utils/include
IDIRS       += $(TIOVX_PATH)/kernels/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/tidl/include
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/include

ifeq ($(HOST_COMPILER),TIARMCGT)
CFLAGS += --display_error_number
CFLAGS += --diag_suppress=179
CFLAGS += --diag_suppress=112
CFLAGS += --diag_suppress=552
endif

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-format-security
endif

include $(FINALE)

endif

endif
