TARGET      := vx_tidl_rt
TARGETTYPE  := dsmo
ifeq ($(BUILD_WITH_OPENACC),yes)
CFLAGS += -fPIC #-Wno-int-to-pointer-cast -Wno-stringop-truncation -Wno-format-overflow
else
CFLAGS += -fPIC -Wno-int-to-pointer-cast -Wno-stringop-truncation -Wno-format-overflow
endif
CPPFLAGS += -fPIC --std=c++11

CSOURCES    += ../tidl_rt_ovx.c
CSOURCES    += ../tidl_rt_utils.c
CSOURCES    += ../tidl_rt_ovx_debug_utils.c
CSOURCES    += ../tidl_rt_profile.c
CPPSOURCES  += ../tidl_rt_ovx_datamove.cpp

ifneq ($(TARGET_PLATFORM), PC)
ifeq ($(TARGET_SOC), $(filter $(TARGET_SOC), J721E j721e J721S2 j721s2 J784S4 j784s4 J742S2 j742s2 AM62A am62a J722S j722s TDA54 tda54))
CSOURCES    += ../tvm_rt_ovx.c
endif
endif

ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
DEFS+=ENABLE_SDK_9_2_COMPATIBILITY
else ifeq ($(ENABLE_SDK_10_0_COMPATIBILITY), 1)
DEFS+=ENABLE_SDK_10_0_COMPATIBILITY
else ifeq ($(ENABLE_SDK_10_1_COMPATIBILITY), 1)
DEFS+=ENABLE_SDK_10_1_COMPATIBILITY
else ifeq ($(ENABLE_SDK_11_0_COMPATIBILITY), 1)
DEFS+=ENABLE_SDK_11_0_COMPATIBILITY
else ifeq ($(ENABLE_SDK_11_1_COMPATIBILITY), 1)
DEFS+=ENABLE_SDK_11_1_COMPATIBILITY
endif


ifeq ($(CODE_COVERAGE_ENABLED_FOR_TIDL), yes)
DEFS+= CODE_COVERAGE_ENABLED_FOR_TIDL
endif

ifeq ($(CODE_COVERAGE_ENABLED_FOR_RT), yes)
CFLAGS+= --no-warnings
endif

IDIRS :=
IDIRS += $(IVISION_PATH)
IDIRS += $(TIOVX_PATH)/include
IDIRS += $(TIOVX_PATH)/kernels/include
IDIRS += $(TIDL_PATH)/arm-tidl/tiovx_kernels/include
IDIRS += $(TIOVX_PATH)/utils/include
IDIRS += $($(_MODULE)_SDIR)/../../../inc
IDIRS += $($(_MODULE)_SDIR)/../../../rt/inc
IDIRS += $(VISION_APPS_PATH)
IDIRS += $(APP_UTILS_PATH)

