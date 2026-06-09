
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C71 C7120 C7504 C7524 C7604))

include $(PRELUDE)
TARGET      := vx_target_kernels_tidl
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(TIOVX_PATH)/kernels/ivision/include
IDIRS       += $(TIOVX_PATH)/kernels/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(IVISION_PATH)
IDIRS       += $(ARM_TIDL_PATH)/../ti_dl/inc
IDIRS       += $(ARM_TIDL_PATH)/../ti_dl/custom
IDIRS       += $(ARM_TIDL_PATH)/tiovx_kernels/include
IDIRS       += $(ARM_TIDL_PATH)/tiovx_kernels/tidl/include
IDIRS       += $(ARM_TIDL_PATH)/rt/inc

IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(APP_UTILS_PATH)/utils/file_io/include
IDIRS       += $(APP_UTILS_PATH)/utils/udma/include


LDIRS       = $(APP_UTILS_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
SYS_STATIC_LIBS += app_utils_file_io

ifeq ($(RTOS_SDK),mcu_sdk)
  # Ensure MCU_SDK_PATH is defined for TDA54
  ifeq ($(MCU_SDK_PATH),)
    MCU_SDK_PATH := $(PSDK_INSTALL_PATH)/mcu_sdk
  endif
  # MCU_SDK source for device includes, DMA_UTILS_PATH for dmautils includes
  IDIRS       += $(MCU_SDK_PATH)/source
  IDIRS       += $(DMA_UTILS_PATH)
  IDIRS       += $(DMA_UTILS_PATH)/include
  IDIRS       += $(DMA_UTILS_PATH)/csl
  IDIRS       += $(DMA_UTILS_PATH)/udma_standalone/include
  DEFS        += MCU_SDK
else ifeq ($(RTOS_SDK),mcu_plus_sdk)
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
  IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
  IDIRS       += $(DMA_UTILS_PATH)
  DEFS        += MCU_PLUS_SDK
endif

ifeq ($(RTOS_SDK),pdk)
  IDIRS       += $(PDK_PATH)
  IDIRS       += $(PDK_PATH)/ti/drv
  IDIRS       += $(PDK_PATH)/ti/drv/udma
  IDIRS       += $(PDK_PATH)/ti/osal
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION -D_TMS320C6X

ifeq ($(RTOS_SDK),mcu_sdk)
CFLAGS      += -DMCU_SDK
else ifeq ($(RTOS_SDK),mcu_plus_sdk)
CFLAGS      += -DMCU_PLUS_SDK
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
DEFS        += __aarch64__
endif

ifeq ($(CODE_COVERAGE_ENABLED_FOR_RT), yes)
CFLAGS+= --no-warnings
endif

ifeq ($(GCOV_ENABLED), 1)
    $(_MODULE)_COPT += -fprofile-arcs
    $(_MODULE)_COPT += -ftest-coverage
endif

endif

include $(FINALE)

endif
