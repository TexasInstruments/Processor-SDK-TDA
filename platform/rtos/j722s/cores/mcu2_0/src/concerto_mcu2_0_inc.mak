DEFS+=CPU_mcu2_0
DEFS+=BUILD_MAIN_R5
DEFS+=SOC_J722S

#DEFS+=BUILD_MCU
#DEFS+=VIM_DIRECT_REGISTRATION

# This enables ARM Thumb mode which reduces firmware size and enables faster boot
#COPT +=--code_state=16
CSOURCES += generated/ti_board_config.c
CSOURCES += generated/ti_board_open_close.c
CSOURCES += generated/ti_dpl_config.c
CSOURCES += generated/ti_drivers_config.c
CSOURCES += generated/ti_drivers_open_close.c
CSOURCES += generated/ti_pinmux_config.c
CSOURCES += generated/ti_power_clock_config.c
ifeq ($(BUILD_ENABLE_ETHFW),yes)
  CSOURCES += generated/ti_enet_config.c
  CSOURCES += generated/ti_enet_open_close.c
  CSOURCES += generated/ti_enet_soc.c
  CSOURCES += generated/ti_enet_lwipif.c
  CSOURCES += generated/ti_enet_init.c
  CSOURCES += generated/ti_enet_dma_init.c
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_$(RTOS_TYPE).cmd
LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map_$(RTOS_TYPE).cmd

IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)

ifeq ($(RTOS),SAFERTOS)
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/api/$(SAFERTOS_ISA_EXT_r5f)
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/api/PrivWrapperStd
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/config
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/kernel/include_api
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/kernel/include_prv
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/queue_registry
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/portable/$(SAFERTOS_ISA_EXT_r5f)
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f/portable/$(SAFERTOS_ISA_EXT_r5f)/$(SAFERTOS_COMPILER_EXT_r5f)
endif

IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/generated

# These are required to be added because cpsw module is added in freertos.syscfg
# regardless of ethfw flag is set or not and generated files will include
# enet related header files

IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-stack/src/include
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-port/include
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-port/freertos/include
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lwip-config/j722s

IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/include
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/include/core
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/utils/include
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/core/lwipif/inc
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/soc/k3/j722s
IDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/hw_include

IDIRS += $(MCU_PLUS_SDK_PATH)/source/board/ethphy/port
IDIRS += $(MCU_PLUS_SDK_PATH)/source/board/ethphy/enet/rtos_drivers/include

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  IDIRS += $(ETHFW_PATH)
endif

LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/$(RTOS_TYPE)/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa/lib

include $($(_MODULE)_SDIR)/../../../concerto_r5f_inc.mak

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  DEFS+=ENABLE_ETHFW
endif

# TODO: Add networking library paths if/when available for MCU Plus SDK J722S
# For now, this is a placeholder - networking libs need to be ported
ifeq ($(BUILD_ENABLE_ETHFW),yes)
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/enet/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/lwip/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/ethfw/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/tsn/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/networking/tsn/tsn-stack/license_lib/
endif

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_mcu2_0
ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_mcu2_0
endif

SYS_STATIC_LIBS += app_utils_hwa
SYS_STATIC_LIBS += app_utils_csi
SYS_STATIC_LIBS += app_utils_sciclient

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  SYS_STATIC_LIBS += app_utils_ethfw
endif

ifeq ($(BUILD_MCU_BOARD_DEPENDENCIES),yes)
  SYS_STATIC_LIBS += app_utils_dss
endif

ADDITIONAL_STATIC_LIBS += drivers.j722s.main-r5f.ti-arm-clang.${TARGET_BUILD}.lib
ADDITIONAL_STATIC_LIBS += vhwa.j722s.main-r5fss0-0.ti-arm-clang.${TARGET_BUILD}.lib

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  ADDITIONAL_STATIC_LIBS += board.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += enet-cpsw.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += lwipif-cpsw-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += lwip-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += lwip-contrib-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += ethfw.j722s.main-r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += tsn_combase-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += tsn_unibase-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += tsn_gptp-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += tsn_uniconf-freertos.j722s.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += yangemb-freertos.j722s.main-r5f.ti-arm-clang.lib
endif
