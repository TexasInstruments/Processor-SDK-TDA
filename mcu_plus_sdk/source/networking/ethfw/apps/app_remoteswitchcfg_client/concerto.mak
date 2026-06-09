########################################################################

ifeq ($(BUILD_CPU_MCU2_1),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_1

# This name becomes the suffix of final _MODULE name
_MODULE=normal

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_client
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

CSOURCES    := main.c
CSOURCES    += virtnetif_lwipif.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES =  $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_remotecfg_client
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract

# TODO: Client app should be agnostic of port specifics
ifeq ($(TARGET_PLATFORM),J7200)
  DEFS += ENABLE_QSGMII_PORTS
endif

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
    DEFS += ETHFW_VEPA_SUPPORT
    DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
  else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
    DEFS += ETHAPP_ENABLE_INTERCORE_ETH
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

ifeq ($(ETHFW_MTS_DEMO_TEST),yes)
  DEFS += ETHFW_MTS_DEMO_TEST
endif

ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_1),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_1

# This name becomes the suffix of final _MODULE name
_MODULE=qnx

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_client_qnx
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

DEFS+=A72_QNX_OS

CSOURCES    := main.c
CSOURCES    += virtnetif_lwipif.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES =  $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_remotecfg_client
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract

# TODO: Client app should be agnostic of port specifics
ifeq ($(TARGET_PLATFORM),J7200)
  DEFS += ENABLE_QSGMII_PORTS
endif

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
endif

# Intercore virtual Ethernet currently not supported in QNX

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

ifeq ($(ETHFW_MTS_DEMO_TEST),yes)
  DEFS += ETHFW_MTS_DEMO_TEST
endif

# iperf server support
ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
  DEFS += ETHFW_VEPA_SUPPORT
  DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU3_0),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu3_0

# This name becomes the suffix of final _MODULE name
_MODULE=$(CPU_ID)

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_client_mcu_3_0
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

CSOURCES    := main.c
CSOURCES    += virtnetif_lwipif.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES =  $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_remotecfg_client
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract

# TODO: Client app should be agnostic of port specifics
ifeq ($(TARGET_PLATFORM),J7200)
  DEFS += ENABLE_QSGMII_PORTS
endif

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(ETHFW_CPSW_VEPA_SUPPORT), yes)
    DEFS += ETHFW_VEPA_SUPPORT
    DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
  else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
    DEFS += ETHAPP_ENABLE_INTERCORE_ETH
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

ifeq ($(ETHFW_MTS_DEMO_TEST),yes)
  DEFS += ETHFW_MTS_DEMO_TEST
endif

ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

ifeq ($(ETHFW_RTOS_MCU3_0_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU3_0
endif

ifeq ($(ETHFW_RTOS_MCU2_1_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU2_1_SERVER
  DEFS += ETHFW_RTOS_MCU3_0 
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif