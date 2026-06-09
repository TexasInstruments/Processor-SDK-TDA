include $(PRELUDE)
TARGET      := ethfw_remotecfg_server
TARGETTYPE  := library
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
TARGET_SOC_FOLDER := $(call lowercase,$(TARGET_PLATFORM))

# Source code files
ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
  CSOURCES += ethfw_vepa.c
endif
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
    ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ethfw_tsn.c
    endif
endif
CSOURCES += cpsw_proxy_server.c
CSOURCES += ethfw_mcast.c
CSOURCES += ethfw_vlan.c
CSOURCES += ethfw_arp.c
CSOURCES += ethfw_api.c
CSOURCES += ethfw_portmirror.c

ifeq ($(ETHFW_IET_ENABLE),yes)
CSOURCES += ethfw_iet.c
endif
ifeq ($(ETHFW_MTS_SUPPORT),yes)
CSOURCES += ts_coupler_server.c
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  CSOURCES += ethfw_monitor.c
endif

#include $(ETHFW_PATH)/apps/concerto_inc.mak

# PDK include path
IDIRS += $(PDK_PATH)/packages
IDIRS += $(PDK_PATH)/packages/ti/drv/enet
IDIRS += $(PDK_PATH)/packages/ti/drv/enet/examples

# lwIP include paths
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/src/include
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/${TARGET_OS_LC}/include
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config
  IDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/config/${TARGET_SOC_FOLDER}
endif

# gPTP stack include paths
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    IDIRS += $(PDK_PATH)/packages/ti/transport/tsn/tsn-stack
    IDIRS += $(PDK_PATH)/packages/ti/transport/tsn/tsn-stack/tsn_combase/tilld/jacinto
  endif
endif

# Top-level ETHFW include path
IDIRS += $(ETHFW_PATH)
IDIRS += ${ETHFW_PATH}/utils/ethfw_abstract/jacinto

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Feature flags: VEPA, Proxy ARP
ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
  DEFS += ETHFW_VEPA_SUPPORT
  DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
else ifeq ($(ETHFW_PROXY_ARP_SUPPORT),yes)
  DEFS += ETHFW_PROXY_ARP_HANDLING
endif

# Feature flags: ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

# Feature flags: ETHFW Monitor
ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

ifeq ($(ETHFW_IET_ENABLE),yes)
  DEF += ETHFW_IET_ENABLE
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

# Feature flags: ETHFW EST demo - should be supported with gPTP
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_GPTP_SUPPORT),yes)
    DEFS += ETHFW_EST_DEMO_SUPPORT
  endif
endif

# To check whether server is running on MCU2_1 or MCU2_0
ifeq ($(ETHFW_RTOS_MCU2_1_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU2_1_SERVER
endif

ETHREMOTECFG_ETHSWITCH_VERSION_LAST_COMMIT := ${shell cd ${ETHFW_PATH};git rev-parse --short=8 HEAD 2>/dev/null}
DEFS += ETHREMOTECFG_ETHSWITCH_VERSION_LAST_COMMIT="\"${ETHREMOTECFG_ETHSWITCH_VERSION_LAST_COMMIT}\""

include $(FINALE)
