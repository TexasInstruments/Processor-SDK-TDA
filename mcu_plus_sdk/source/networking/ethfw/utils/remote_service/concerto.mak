
include $(PRELUDE)
TARGET      := app_remote_service
TARGETTYPE  := library

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 AM65XX J742S2))
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F R5Ft C66 C71))
  CSOURCES := src/app_remote_service.c
endif
endif
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

IDIRS       := ${ETHFW_PATH}
IDIRS       += $(PDK_PATH)/packages

include $(FINALE)

