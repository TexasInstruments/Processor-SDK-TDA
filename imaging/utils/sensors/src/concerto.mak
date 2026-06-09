ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS THREADX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),R5F))

include $(PRELUDE)
TARGET      := app_utils_sensors
TARGETTYPE  := library

ifneq ($(SOC)$(TARGET_OS),j722sQNX)
CSOURCES    := app_sensors.c
endif

include $(FINALE)

endif
endif
