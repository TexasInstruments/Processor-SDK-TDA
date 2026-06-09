ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 A53 R5F A720))

include $(PRELUDE)
TARGET      := vx_nested_kernels_tidl
TARGETTYPE  := library
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J784S4 j784s4 J722S j722s J742S2 j742s2))
CSOURCES    := $(call all-c-files)
endif
IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
IDIRS       += $(TIOVX_PATH)/kernels/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/tidl/include
IDIRS       += $(TIDL_TIOVX_KERNELS_PATH)/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(TIDL_RT_PATH)/inc
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(APP_UTILS_PATH)/utils/file_io/include

LDIRS       = $(APP_UTILS_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
SYS_STATIC_LIBS += app_utils_file_io

ifeq ($(HOST_COMPILER),$(filter $(HOST_COMPILER),GCC GCC_LINARO GCC_WINDOWS GCC_LINUX GCC_LINUX_ARM GCC_QNX_ARM))
CFLAGS += -Wno-unused-function
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS += -DHOST_EMULATION
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
