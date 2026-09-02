ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 R5F))

include $(PRELUDE)
TARGET      := app_utils_csi
TARGETTYPE  := library

ifeq ($(TARGET_CPU),R5F)
CSOURCES    := app_csi.c
endif

ifeq ($(RTOS_SDK), mcu_plus_sdk)
IDIRS       += $(MCU_PLUS_SDK_PATH)/source
IDIRS       += $(MCU_PLUS_SDK_PATH)/source/drivers
else
IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(PDK_PATH)/packages/ti/drv
endif

IDIRS += $(HOST_ROOT)
IDIRS += $(APP_UTILS_PATH)

ifeq ($(SOC), am62a)
SKIPBUILD=1
endif

include $(FINALE)

endif