ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120 C7504 C7524 C7604))

include $(PRELUDE)
TARGET      := vx_target_kernels_tvm
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIOVX_PATH)/source/include
IDIRS       += $(TIOVX_PATH)/include
IDIRS       += $(TIOVX_PATH)/kernels/include
IDIRS       += $(ARM_TIDL_PATH)/../ti_dl/inc
IDIRS       += $(ARM_TIDL_PATH)/../ti_dl/custom
IDIRS       += $(ARM_TIDL_PATH)/tiovx_kernels/include
IDIRS       += $(ARM_TIDL_PATH)/tiovx_kernels/tidl/include
IDIRS       += $(ARM_TIDL_PATH)/rt/inc

ifeq ($(RTOS_SDK),mcu_sdk)
	# Ensure MCU_SDK_PATH is defined for TDA54
	ifeq ($(MCU_SDK_PATH),)
		MCU_SDK_PATH := $(PSDK_INSTALL_PATH)/mcu_sdk
	endif
	# SDK source paths needed for proper driver includes
	IDIRS       += $(MCU_SDK_PATH)/source
	IDIRS       += $(MCU_SDK_PATH)/source/compatibility/dpl/include
	IDIRS       += $(MCU_SDK_PATH)/source/arch/include
	IDIRS       += $(MCU_SDK_PATH)/source/arch/c76/include
	IDIRS       += $(MCU_SDK_PATH)/source/compiler/c76-ti-c7000
	IDIRS       += $(MCU_SDK_PATH)/source/device/tda54/include
	IDIRS       += $(MCU_SDK_PATH)/source/device/tda54/include/hw
	IDIRS       += $(MCU_SDK_PATH)/ti_sdk_config/tda54/default/Hal_Cfg
	IDIRS       += $(MCU_SDK_PATH)/source/drivers
	IDIRS       += $(DMA_UTILS_PATH)
else ifeq ($(RTOS_SDK),mcu_plus_sdk)
	IDIRS       += $(MCU_PLUS_SDK_PATH)/source
	IDIRS       += $(MCU_PLUS_SDK_PATH)/source/kernel/dpl
else ifeq ($(RTOS_SDK),pdk)
	IDIRS       += $(PDK_PATH)
	IDIRS       += $(PDK_PATH)/ti/osal
endif

include $(FINALE)

endif
