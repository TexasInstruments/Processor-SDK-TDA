include $(PRELUDE)

TARGET      := ethfw_common
TARGETTYPE  := library
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
TARGET_SOC_FOLDER := $(call lowercase,$(TARGET_PLATFORM))

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 J742S2))
  ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
    CSOURCES := ethfw_trace.c ethfw_timer.c

    IDIRS := ${ETHFW_PATH}
    IDIRS += $(PDK_PATH)/packages
    IDIRS += $(PDK_PATH)/packages/ti/drv/enet
    IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto

    DEFS += MAKEFILE_BUILD
  endif
endif

include $(FINALE)