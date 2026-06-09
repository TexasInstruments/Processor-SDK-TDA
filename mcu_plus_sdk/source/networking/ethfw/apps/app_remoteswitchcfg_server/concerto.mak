########################################################################

ifeq ($(BUILD_CPU_MCU2_0),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_0

# This name becomes the suffix of final _MODULE name
_MODULE=normal

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_server
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES = $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_callbacks
STATIC_LIBS += eth_intervlan
STATIC_LIBS += ethfw_board
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract
STATIC_LIBS += ethfw_remotecfg_server
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
STATIC_LIBS += ethfw_estdemo
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Comment out to disable QSGMII ports in J721E EVM
DEFS += ENABLE_QSGMII_PORTS

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(TARGET_PLATFORM),J7200)
    ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
  else
    ENET_APPUTILS_LIB = enet_example_utils_full_$(TARGET_OS_LC)
  endif
endif

# Intercore virtual Ethernet
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
    DEFS += ETHFW_VEPA_SUPPORT
    DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
  else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
    DEFS += ETHAPP_ENABLE_INTERCORE_ETH
  endif
endif

# ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

# iperf server support
ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

ifeq ($(ETHFW_RTOS_MCU3_0_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU3_0
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_0),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_0

# This name becomes the suffix of final _MODULE name
_MODULE=qnx

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_server_qnx
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

DEFS+=A72_QNX_OS

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES = $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_callbacks
STATIC_LIBS += eth_intervlan
STATIC_LIBS += ethfw_board
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract
STATIC_LIBS += ethfw_remotecfg_server
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
STATIC_LIBS += ethfw_estdemo
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Comment out to disable QSGMII ports in J721E EVM
DEFS += ENABLE_QSGMII_PORTS

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(TARGET_PLATFORM),J7200)
    ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
  else
    ENET_APPUTILS_LIB = enet_example_utils_full_$(TARGET_OS_LC)
  endif
endif

# Intercore virtual Ethernet currently not supported in QNX

# ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

ifeq ($(ETHFW_IET_ENABLE),yes)
  DEFS += ETHFW_IET_ENABLE
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

# iperf server support
ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

ifeq ($(ETHFW_RTOS_MCU3_0_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU3_0
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_0),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_0

# This name becomes the suffix of final _MODULE name
_MODULE=ccs

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_server_ccs
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

# Needed to identify the type of image being built
DEFS        += ETHFW_CCS

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES = $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_callbacks
STATIC_LIBS += eth_intervlan
STATIC_LIBS += ethfw_board
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract
STATIC_LIBS += ethfw_remotecfg_server
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
STATIC_LIBS += ethfw_estdemo
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Comment out to disable QSGMII ports in J721E EVM
DEFS += ENABLE_QSGMII_PORTS

# Enable MAC-only ports. Comment out for switch only ports
# Disabled in profiling setting as there is single port
ifneq ($(ETHFW_BOOT_TIME_PROFILING),yes)
  DEFS += ENABLE_MAC_ONLY_PORTS
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ENET_APPUTILS_LIB = enet_example_utils_full_$(TARGET_OS_LC)
endif

# Intercore virtual Ethernet
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
    DEFS += ETHFW_VEPA_SUPPORT
    DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
  else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
    DEFS += ETHAPP_ENABLE_INTERCORE_ETH
  endif
endif

# ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# Feature flags: ETHFW MTS support
ifeq ($(ETHFW_MTS_SUPPORT),yes)
  DEFS += ETHFW_MTS_SUPPORT
endif

ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

# ETHFW boot time profiling
ifeq ($(ETHFW_BOOT_TIME_PROFILING),yes)
  DEFS += ETHFW_BOOT_TIME_PROFILING
endif

# Ethfw Intervlan demo
ifeq ($(ETHFW_DEMO_SUPPORT),yes)
  DEFS += ETHFW_DEMO_SUPPORT
endif

# Feature flags: ETHFW EST demo - should be supported with gPTP
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_GPTP_SUPPORT),yes)
    DEFS += ETHFW_EST_DEMO_SUPPORT
  endif
endif

ifeq ($(ETHFW_RTOS_MCU3_0_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU3_0
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_1_SERVER),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_1

# This name becomes the suffix of final _MODULE name
_MODULE=normal

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_server_mcu2_1
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES = $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_callbacks
STATIC_LIBS += eth_intervlan
STATIC_LIBS += ethfw_board
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract
STATIC_LIBS += ethfw_remotecfg_server
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
STATIC_LIBS += ethfw_estdemo
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Comment out to disable QSGMII ports in J721E EVM
DEFS += ENABLE_QSGMII_PORTS

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

# MCU3_0 will be enabled by default if the server is MCU2_1
DEFS += ETHFW_RTOS_MCU3_0

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(TARGET_PLATFORM),J7200)
    ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
  else
    ENET_APPUTILS_LIB = enet_example_utils_full_$(TARGET_OS_LC)
  endif
endif

# Intercore virtual Ethernet
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
    DEFS += ETHFW_VEPA_SUPPORT
    DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
  else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
    DEFS += ETHAPP_ENABLE_INTERCORE_ETH
  endif
endif

# ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# iperf server support
ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

ifeq ($(ETHFW_RTOS_MCU2_1_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU2_1_SERVER
  DEFS += ETHFW_RTOS_MCU3_0
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_1_SERVER),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_1

# This name becomes the suffix of final _MODULE name
_MODULE=qnx

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_server_qnx_mcu2_1
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

DEFS+=A72_QNX_OS

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES = $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_callbacks
STATIC_LIBS += eth_intervlan
STATIC_LIBS += ethfw_board
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract
STATIC_LIBS += ethfw_remotecfg_server
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
STATIC_LIBS += ethfw_estdemo
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Comment out to disable QSGMII ports in J721E EVM
DEFS += ENABLE_QSGMII_PORTS

# Enable MAC-only ports. Comment out for switch only ports
DEFS += ENABLE_MAC_ONLY_PORTS

# MCU3_0 will be enabled by default if the server is MCU2_1
DEFS += ETHFW_RTOS_MCU3_0

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(TARGET_PLATFORM),J7200)
    ENET_APPUTILS_LIB = enet_example_utils_$(TARGET_OS_LC)
  else
    ENET_APPUTILS_LIB = enet_example_utils_full_$(TARGET_OS_LC)
  endif
endif

# Intercore virtual Ethernet currently not supported in QNX

# ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

# iperf server support
ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

ifeq ($(ETHFW_RTOS_MCU2_1_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU2_1_SERVER
  DEFS += ETHFW_RTOS_MCU3_0
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2_1_SERVER),yes)
ifneq (,$(filter $(TARGET_CPU),R5F R5Ft))

CPU_ID=mcu2_1

# This name becomes the suffix of final _MODULE name
_MODULE=ccs

include $(PRELUDE)

TARGET      := app_remoteswitchcfg_server_ccs_mcu2_1
TARGETTYPE  := exe
TARGET_OS_LC := $(call lowercase,$(TARGET_OS))
SOC_LC      := $(call lowercase,$(TARGET_PLATFORM))

# Needed to identify the type of image being built
DEFS        += ETHFW_CCS

CSOURCES    := main.c
ifeq ($(TARGET_OS),FREERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_default.c
else ifeq ($(TARGET_OS),SAFERTOS)
  CSOURCES += ../ipc_cfg/ipc_trace.c
  CSOURCES += ../common/r5f_mpu_$(SOC_LC)_safertos.c
endif

LINKER_CMD_FILES = $(SDIR)/$(SOC_LC)/$(CPU_ID)/linker_mem_map.cmd
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  LINKER_CMD_FILES += $(SDIR)/linker_$(TARGET_OS_LC)_$(CPU_ID).cmd
endif

STATIC_LIBS += ethfw_callbacks
STATIC_LIBS += eth_intervlan
STATIC_LIBS += ethfw_board
STATIC_LIBS += ethfw_common
STATIC_LIBS += ethfw_abstract
STATIC_LIBS += ethfw_remotecfg_server
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
STATIC_LIBS += ethfw_estdemo
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  DEFS += MAKEFILE_BUILD
endif

# Comment out to disable QSGMII ports in J721E EVM
DEFS += ENABLE_QSGMII_PORTS

# MCU3_0 will be enabled by default if the server is MCU2_1
DEFS += ETHFW_RTOS_MCU3_0

# Enable MAC-only ports. Comment out for switch only ports
# Disabled in profiling setting as there is single port
ifneq ($(ETHFW_BOOT_TIME_PROFILING),yes)
  DEFS += ENABLE_MAC_ONLY_PORTS
endif

ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ENET_APPUTILS_LIB = enet_example_utils_full_$(TARGET_OS_LC)
endif

# Intercore virtual Ethernet
ifneq ($(filter $(TARGET_OS),FREERTOS SAFERTOS),)
  ifeq ($(ETHFW_CPSW_VEPA_SUPPORT),yes)
    DEFS += ETHFW_VEPA_SUPPORT
    DEFS += ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA
  else ifeq ($(ETHFW_INTERCORE_ETH_SUPPORT),yes)
    DEFS += ETHAPP_ENABLE_INTERCORE_ETH
  endif
endif

# ETHFW gPTP stack - for now, supported in FreeRTOS only
ifeq ($(ETHFW_GPTP_SUPPORT),yes)
  ifeq ($(TARGET_OS),FREERTOS)
    DEFS += ETHFW_GPTP_SUPPORT
  endif
endif

ifeq ($(ETHFW_MONITOR_SUPPORT),yes)
  DEFS += ETHFW_MONITOR_SUPPORT
endif

ifeq ($(ETHFW_IPERF_SERVER_SUPPORT),yes)
  DEFS += ETHAPP_ENABLE_IPERF_SERVER
endif

# ETHFW boot time profiling
ifeq ($(ETHFW_BOOT_TIME_PROFILING),yes)
  DEFS += ETHFW_BOOT_TIME_PROFILING
endif

# Ethfw Intervlan demo
ifeq ($(ETHFW_DEMO_SUPPORT),yes)
  DEFS += ETHFW_DEMO_SUPPORT
endif

# Feature flags: ETHFW EST demo - should be supported with gPTP
ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
  ifeq ($(ETHFW_GPTP_SUPPORT),yes)
    DEFS += ETHFW_EST_DEMO_SUPPORT
  endif
endif

ifeq ($(ETHFW_RTOS_MCU2_1_SUPPORT),yes)
  DEFS += ETHFW_RTOS_MCU2_1_SERVER
  DEFS += ETHFW_RTOS_MCU3_0
endif

include $(ETHFW_PATH)/apps/concerto_inc.mak

include $(FINALE)

endif
endif

########################################################################
