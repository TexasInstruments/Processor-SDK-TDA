include $(PRELUDE)

TARGET      := ethfw_abstract
TARGETTYPE  := library
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
TARGET_SOC_FOLDER := $(call lowercase,$(TARGET_PLATFORM))

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 AM65XX J742S2))
  ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
    CSOURCES += jacinto/ethfw_osal.c
    CSOURCES += jacinto/ethfw_ipc.c

    IDIRS := ${ETHFW_PATH}
    IDIRS += $(PDK_PATH)/packages
    IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto

    DEFS += MAKEFILE_BUILD
  endif
endif

include $(FINALE)
