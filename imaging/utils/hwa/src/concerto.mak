ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 R5F))

include $(PRELUDE)

TARGET      := app_utils_hwa
TARGETTYPE  := library

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64))
CSOURCES += app_hwa_api_x86.c
endif

ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
CSOURCES    := app_hwa_hlos.c
endif

ifeq ($(TARGET_CPU),R5F)
ifneq ($(SOC), $(filter $(SOC), am62a j722s))
CSOURCES    := app_hwa.c app_hwa_firewall.c
else
CSOURCES    := app_hwa.c
ifeq ($(SOC), am62a)
CSOURCES	+= app_csi.c
endif
endif
endif

ifeq ($(RTOS_SDK), mcu_plus_sdk)
IDIRS       += $(MCU_PLUS_SDK_PATH)/source
IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa
else
IDIRS       += $(PDK_PATH)/packages/ti/drv
IDIRS       += $(PDK_PATH)/packages/ti/drv/vhwa
endif

IDIRS += $(HOST_ROOT)/kernels/include
IDIRS += $(HOST_ROOT)/utils/hwa/include

include $(FINALE)

endif
