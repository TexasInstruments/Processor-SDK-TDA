########################################################################

ifeq ($(BUILD_CPU_MCU2_1),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_1

# This name becomes the suffix of final _MODULE name
_MODULE=normal

include $(PRELUDE)

TARGET      := client_test_app
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../../ipc_cfg/ipc_trace.c
  CSOURCES += r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../../ipc_cfg/ipc_trace.c
  CSOURCES += r5f_mpu_$(SOC_LC)_safertos.c
endif

CSOURCES += ../../test_cases/test_connection.c
CSOURCES += ../../test_cases/test_resources.c

# Enable routing of Client App logs and traces to UART
# DEFS += ENABLE_UART_LOG

IDIRS += $(ETHFW_PATH)/unit_test/test_cases

LINKER_CMD_FILES =  $(SDIR)/$(SOC_LC)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC).cmd
endif

STATIC_LIBS += ethfw_remotecfg_client
STATIC_LIBS += ethfw_common
STATIC_LIBS += unity_console
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

ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

include $(ETHFW_PATH)/unit_test/concerto_inc.mak

include $(FINALE)

endif
endif