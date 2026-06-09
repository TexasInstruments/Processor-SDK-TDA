
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72 A53 A720))

include $(PRELUDE)
TARGET      := vx_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/tidl/include
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/include
IDIRS       += $(TIOVX_PATH)/kernels/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_RT_PATH)/inc

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
endif

ifeq ($(CODE_COVERAGE_ENABLED_FOR_RT), yes)
CFLAGS+= --no-warnings
endif

ifeq ($(GCOV_ENABLED), 1)
    $(_MODULE)_COPT += -fprofile-arcs
    $(_MODULE)_COPT += -ftest-coverage
endif

include $(FINALE)

endif
