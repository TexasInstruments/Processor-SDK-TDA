include $(PRELUDE)

TARGET      := ethfw_callbacks
TARGETTYPE  := library
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
TARGET_SOC_FOLDER := $(call lowercase,$(TARGET_PLATFORM))

ifneq (,$(filter $(TARGET_PLATFORM),J721E J7200 J784S4 AM65XX J742S2))
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  CSOURCES := src/ethfw_callbacks_lwipif.c
endif

IDIRS := ${ETHFW_PATH}
IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/src/include
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/$(TARGET_OS_LC)/include
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config/$(TARGET_SOC_FOLDER)
endif
IDIRS += $(PDK_PATH)/packages
IDIRS += $(PDK_PATH)/packages/ti/drv/enet
IDIRS += $(PDK_PATH)/packages/ti/drv/enet/examples
IDIRS += $(PDK_PATH)/packages/ti/drv/enet/lwipif/inc

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
  DEFS += ETHFW_VEPA_SUPPORT
  DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
else ifeq ($(ETHFW_PROXY_ARP_SUPPORT),yes)
  DEFS += ETHFW_PROXY_ARP_HANDLING
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

endif

include $(FINALE)
