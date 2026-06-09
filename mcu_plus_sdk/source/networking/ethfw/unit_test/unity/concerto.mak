########################################################################

ifneq (,$(filter yes,$(BUILD_CPU_MCU2_1) $(BUILD_CPU_MCU3_0)))
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

include $(PRELUDE)
TARGET      := unity_console
TARGETTYPE  := library


ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 J742S2))
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R5Ft))
  CSOURCES := src/unity.c
endif
endif
endif

IDIRS += $(ETHFW_PATH)/unit_test/unity/include

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_0),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu2_0

_MODULE=$(CPU_ID)

include $(PRELUDE)
TARGET      := unity_uart
TARGETTYPE  := library

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 J742S2))
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R5Ft))
  CSOURCES := src/unity.c
endif
endif
endif

# Enable routing of Unity prints to UART
DEFS += UNITY_INCLUDE_CONFIG_H

IDIRS += $(ETHFW_PATH)/unit_test/unity/include
IDIRS += $(PDK_PATH)/packages

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_1_SERVER),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu2_1

_MODULE=$(CPU_ID)

include $(PRELUDE)
TARGET      := unity_uart
TARGETTYPE  := library

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 J742S2))
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R5Ft))
  CSOURCES := src/unity.c
endif
endif
endif

# Enable routing of Unity prints to UART
DEFS += UNITY_INCLUDE_CONFIG_H

IDIRS += $(ETHFW_PATH)/unit_test/unity/include
IDIRS += $(PDK_PATH)/packages

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

include $(FINALE)

endif
endif