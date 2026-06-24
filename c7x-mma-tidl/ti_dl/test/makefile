ALGBASE_PATH ?= $(abspath ../../)
PSDK_INSTALL_PATH ?= $(abspath ../../../)
include $(ALGBASE_PATH)/makerules/config.mk

GCC_LINUX_ROOT := /usr

CGT7X_ROOT := $(DSP_TOOLS)
PDK_PATH   := $(PDK_INSTALL_PATH)

ifeq ($(TARGET_BUILD),debug)
  PROFILE := debug
else
  PROFILE := release
endif

DIRECTORIES := src

TARGET_COMBOS :=

SYSDEFS := CONCERTO_BUILD

ifeq ($(TIDL_BUILD_PATHS), LEGACY)
TARGET_FAMILY ?= C71
else
TARGET_FAMILY ?= $(TARGET_C7X_VERSION)
endif

ifeq ($(TARGET_PLATFORM),TI_DEVICE)
  SYS_XDC_IDIRS = $(BIOS_PATH)/packages
  ifeq ($(TIDL_BUILD_FOR_LOKI), 1)
    TARGET_COMBOS += J7_LOKI:$(RTOS):$(TARGET_FAMILY):1:$(PROFILE):CGT7X
  else
    ifeq ($(TIDL_BIOS_BUILD), 0)
      ifeq ($(TIDL_HOST_CCS), 1)
        TARGET_COMBOS += J7_NONOS:$(RTOS):$(TARGET_FAMILY):1:$(PROFILE):CGT7X
      else
        TARGET_COMBOS += J7_NONOS_SDK:$(RTOS):$(TARGET_FAMILY):1:$(PROFILE):CGT7X
      endif
    else
      ifeq ($(TIDL_HOST_CCS), 1)
        TARGET_COMBOS += J7_CCS:$(RTOS):$(TARGET_FAMILY):1:$(PROFILE):CGT7X
      else
        TARGET_COMBOS += J7_SDK:$(RTOS):$(TARGET_FAMILY):1:$(PROFILE):CGT7X
      endif
    endif
  endif
else
  ifdef SystemRoot
    TARGET_COMBOS += PC:WINDOWS:x64:1:$(PROFILE):CL
  else
    ifeq ($(BUILD_WITH_CUDA),yes)
      TARGET_COMBOS += PC:LINUX:x86_64:1:$(PROFILE):NVCC_LINUX
    else
      ifeq ($(BUILD_WITH_OPENACC),yes)
        TARGET_COMBOS += PC:LINUX:x86_64:1:$(PROFILE):NVCPP_LINUX
      else
        TARGET_COMBOS += PC:LINUX:x86_64:1:$(PROFILE):GCC_LINUX
      endif
    endif
  endif
endif

BUILD_MULTI_PROJECT := 1

include $(CONCERTO_ROOT)/rules.mak

# Project specific rules

.PHONY: all
all:

final_install: all
	$(foreach mod, $(MODULES), $(call TIDL_EXE_COPY,  $($(mod)_TDIR)/$($(mod)_TARGET)$(BIN_EXT), $($(mod)_SDIR)/../.. ))


ifdef SystemRoot
define TIDL_EXE_COPY
	-$(PRINT) Copying built libraries: $(1)   to    $(2)
	-$(Q)if exist $(call PATH_CONV,$(1)) $(COPY) $(call PATH_CONV,$(1)) $(call PATH_CONV,$(2)) > NULL
endef
else
define TIDL_EXE_COPY
	-$(PRINT) Copying built libraries: $(1)   to    $(2)
	-$(Q)$(COPY) $(1) $(2)
endef
endif
