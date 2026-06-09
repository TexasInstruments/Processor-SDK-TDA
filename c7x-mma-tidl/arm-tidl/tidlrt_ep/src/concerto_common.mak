TARGET      := tidlrt_EP
TARGETTYPE  := dsmo

CPPSOURCES    += ../tidlrt_ep.cpp
CPPSOURCES    += ../../../common/tidl_runtimes_infer_common.cpp

IDIRS += $($(_MODULE)_SDIR)/../../../rt/inc
IDIRS += $(IVISION_PATH)
IDIRS += $($(_MODULE)_SDIR)/../../../common
IDIRS += $($(_MODULE)_SDIR)/../src

LDIRS += $($(_MODULE)_SDIR)/../../../rt/out/$(TARGET_PLATFORM)/$(TARGET_CPU)/LINUX/$(TARGET_BUILD)

SHARED_LIBS += dl

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721E j721e))
BUILD_DEFS += SOC_J721E
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721S2 j721s2))
BUILD_DEFS += SOC_J721S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J784S4 j784s4))
BUILD_DEFS += SOC_J784S4
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J742S2 j742s2))
BUILD_DEFS += SOC_J742S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
BUILD_DEFS += SOC_J722S
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
BUILD_DEFS += SOC_AM62A
else
BUILD_DEFS += SOC_J721E
endif

ifneq ($(BUILD_WITH_OPENACC) ,yes)
CPPFLAGS  += --std=c++11 \
             -Wno-maybe-uninitialized \
             -Wno-unused-variable \
             -Wno-sign-compare \
             -Wno-unused-but-set-variable \
             -Wno-unused-result \
             -Wno-format-overflow \
             -Wno-format-truncation
endif