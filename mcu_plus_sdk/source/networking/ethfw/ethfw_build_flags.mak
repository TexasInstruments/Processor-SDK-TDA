
ifndef $(ETHFW_BUILD_FLAGS_MAK)
ETHFW_BUILD_FLAGS_MAK = 1

#Build log flags
NO_BANNER?=1

BUILD_TARGET_MODE?=yes
BUILD_EMULATION_MODE?=no
# valid values: X86 x86_64 all
BUILD_EMULATION_ARCH?=x86_64

# Build specific CPUs
BUILD_CPU_MPU1?=no
BUILD_CPU_MCU1_0?=no
BUILD_CPU_MCU2_0?=yes
BUILD_CPU_MCU3_0?=no
BUILD_CPU_C6x_1?=no
BUILD_CPU_C6x_2?=no
BUILD_CPU_C7x_1?=no
BUILD_CPU_MCU1_1?=no
BUILD_CPU_MCU2_1?=yes
BUILD_CPU_MCU2_1_SERVER?=no
BUILD_CPU_MCU3_1?=no
BUILD_SOC_LIST ?= J721E J7200 J784S4 J742S2
export BUILD_SOC_LIST
# Build FREERTOS only binaries
BUILD_APP_FREERTOS?=yes
BUILD_APP_SAFERTOS?=no
#Build Profile
PROFILE?=release

# Treat compiler warning as error
# Supported Values: 1 | 0
TREAT_WARNINGS_AS_ERROR ?= 1

# R5F Thumb mode
BUILD_R5F_THUMB?=yes

# Build a specific CPU type's based on CPU flags status defined above
ifneq (,$(filter yes,$(BUILD_CPU_MCU1_0) $(BUILD_CPU_MCU1_1) $(BUILD_CPU_MCU2_0) $(BUILD_CPU_MCU2_1) $(BUILD_CPU_MCU3_0) $(BUILD_CPU_MCU3_1) $(BUILD_CPU_MCU2_1_SERVER)))
  ifeq ($(BUILD_R5F_THUMB),yes)
    BUILD_ISA_R5F=no
    BUILD_ISA_R5Ft=yes
  else
    BUILD_ISA_R5F=yes
    BUILD_ISA_R5Ft=no
  endif
else
  BUILD_ISA_R5F=no
  BUILD_ISA_R5Ft=no
endif

ifneq (,$(filter yes,$(BUILD_CPU_C6x_1) $(BUILD_CPU_C6x_2)))
  BUILD_ISA_C6x=yes
else
  BUILD_ISA_C6x=no
endif

ifneq (,$(filter yes,$(BUILD_CPU_C7x_1)))
  BUILD_ISA_C7x=yes
else
  BUILD_ISA_C7x=no
endif

ifneq (,$(filter yes,$(BUILD_CPU_MPU1)))
  BUILD_ISA_A72=yes
else
  BUILD_ISA_A72=no
endif

ifneq (,$(filter yes,$(BUILD_CPU_MPU1)))
  BUILD_ISA_A53=yes
else
  BUILD_ISA_A53=no
endif

ifneq (,$(filter $(BUILD_SOC_LIST),J784S4 J742S2))
  ETHFW_CPSW_VEPA_SUPPORT?=yes
endif

# Proxy ARP handling support
# Supported Values: yes | no
ifneq (,$(filter yes,$(BUILD_CPU_MCU2_0) $(BUILD_CPU_MCU2_1_SERVER)))
  ETHFW_PROXY_ARP_SUPPORT=yes
endif

# Inter-core virtual ethernet support
# Supported Values: yes | no
ifneq (,$(filter yes,$(BUILD_CPU_MCU2_0) $(BUILD_CPU_MCU2_1) $(BUILD_CPU_MCU3_0) $(BUILD_CPU_MCU2_1_SERVER)))
  ETHFW_INTERCORE_ETH_SUPPORT?=yes
endif

# gPTP stack support - currently supported in FreeRTOS build only
ETHFW_GPTP_SUPPORT?=yes

#IET Support for ETHFW server
ETHFW_IET_ENABLE?=yes

# Enable EthFw Monitor - to monitor the EthFw status and handle any HW Errors if detected.
ETHFW_MONITOR_SUPPORT?=yes

# Enable iperf server (TCP only) by default
ETHFW_IPERF_SERVER_SUPPORT?=yes

# Boot time profile support
ETHFW_BOOT_TIME_PROFILING?=no

# Ethfw Demo support
ETHFW_DEMO_SUPPORT?=no

# Support for random MAC Address generation
ETHFW_RAND_MACADDR_GEN?=yes

# Feature flags: PPS Signal Gen
ETHFW_PPS_DEMO_SUPPORT?=yes

# EthFw EST demo
ETHFW_EST_DEMO_SUPPORT?=no
ETHFW_EST_DEMO_TALKER?=no
ETHFW_EST_DEMO_LISTENER?=no

# EthFw Mutlicore Timesync support
ifneq (,$(filter $(BUILD_SOC_LIST),J784S4 J721E J7200))
  ETHFW_MTS_SUPPORT?=yes
  ETHFW_MTS_DEMO_TEST?=no
endif

# Disable RTOS client build for mcu2_1 core if enabled for mcu3_0 core
ifneq (,$(filter $(BUILD_SOC_LIST),J784S4 J721E J742S2))
  ETHFW_RTOS_MCU3_0_SUPPORT?=no
endif

ifeq ($(ETHFW_RTOS_MCU3_0_SUPPORT),yes)
  BUILD_CPU_MCU2_1=no
  BUILD_CPU_MCU3_0=yes
endif

# Build EthFw server for MCU2_1
ifeq ($(ETHFW_RTOS_MCU2_1_SUPPORT),yes)
  BUILD_CPU_MCU2_1=no
  BUILD_CPU_MCU2_0=no
  BUILD_CPU_MCU2_1_SERVER=yes
  BUILD_CPU_MCU3_0=yes
endif

endif # ifndef $(ETHFW_BUILD_FLAGS_MAK)
