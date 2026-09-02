DEFS+=CPU_mcu1_0
DEFS+=BUILD_WKUP_R5
DEFS+=SOC_J722S
DEFS+=ENABLE_SCICLIENT_DIRECT

# This enables ARM Thumb mode which reduces firmware size and enables faster boot
COPT +=--code_state=16

CSOURCES += generated/ti_board_config.c
CSOURCES += generated/ti_board_open_close.c
CSOURCES += generated/ti_dpl_config.c
CSOURCES += generated/ti_drivers_config.c
CSOURCES += generated/ti_drivers_open_close.c
CSOURCES += generated/ti_pinmux_config.c
CSOURCES += generated/ti_power_clock_config.c

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_$(RTOS_TYPE).cmd
LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map_$(RTOS_TYPE).cmd

IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/generated

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

LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/$(RTOS_TYPE)/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/sciserver/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/rm_pm_hal/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/sciclient_direct/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/self_reset/lib/

include $($(_MODULE)_SDIR)/../../../concerto_r5f_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_mcu1_0

ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_mcu1_0
endif

SYS_STATIC_LIBS += app_utils_sciserver

ADDITIONAL_STATIC_LIBS += sciclient_direct.j722s.wkup-r5f.ti-arm-clang.${TARGET_BUILD}.lib
ADDITIONAL_STATIC_LIBS += sciserver.j722s.wkup-r5f.ti-arm-clang.${TARGET_BUILD}.lib
ADDITIONAL_STATIC_LIBS += drivers.j722s.wkup-r5f.ti-arm-clang.${TARGET_BUILD}.lib
ADDITIONAL_STATIC_LIBS += rm_pm_hal.j722s.wkup-r5f.ti-arm-clang.${TARGET_BUILD}.lib
ADDITIONAL_STATIC_LIBS += self_reset.j722s.wkup-r5f.ti-arm-clang.${TARGET_BUILD}.lib
