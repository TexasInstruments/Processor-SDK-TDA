DEFS+=CPU_mcu1_0
DEFS+=BUILD_MCU1_0
DEFS+=BUILD_MCU
DEFS+=VIM_DIRECT_REGISTRATION
DEFS+=ENABLE_SCICLIENT_DIRECT

# This enables ARM Thumb mode which reduces firmware size and enables faster boot
COPT +=--code_state=16

ifeq ($(RTOS),SYSBIOS)
  XDC_BLD_FILE = $($(_MODULE)_SDIR)/../bios_cfg/config_r5f.bld
  XDC_IDIRS    = $($(_MODULE)_SDIR)/../bios_cfg/
  XDC_CFG_FILE = $($(_MODULE)_SDIR)/mcu1_0.cfg
  XDC_PLATFORM = "ti.platforms.cortexR:J7ES_MCU"
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker.cmd
endif
ifeq ($(RTOS),FREERTOS)
  ifeq ($(RTOS_SDK),pdk)
    CSOURCES += $(SOC)_mpu_cfg.c
    LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
  else
    CSOURCES += generated/ti_board_config.c
    CSOURCES += generated/ti_board_open_close.c
    CSOURCES += generated/ti_dpl_config.c
    CSOURCES += generated/ti_drivers_config.c
    CSOURCES += generated/ti_drivers_open_close.c
    CSOURCES += generated/ti_pinmux_config.c
    CSOURCES += generated/ti_power_clock_config.c
    LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos_mcuplus.cmd
  endif
endif
ifeq ($(RTOS),THREADX)
  CSOURCES += generated/ti_board_config.c
  CSOURCES += generated/ti_board_open_close.c
  CSOURCES += generated/ti_dpl_config.c
  CSOURCES += generated/ti_drivers_config.c
  CSOURCES += generated/ti_drivers_open_close.c
  CSOURCES += generated/ti_pinmux_config.c
  CSOURCES += generated/ti_power_clock_config.c
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_threadx_mcuplus.cmd
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)

ifeq ($(RTOS_SDK),pdk)
  ifeq ($(RTOS),FREERTOS)
    LDIRS += $(PDK_PATH)/packages/ti/kernel/lib/$(SOC)/mcu1_0/$(TARGET_BUILD)/
  endif

  LDIRS += $(PDK_PATH)/packages/ti/drv/ipc/lib/$(SOC)/mcu1_0/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/drv/mailbox/lib/$(SOC)/mcu1_0/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)/mcu1_0/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)/mcu1_0/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/drv/vhwa/lib/$(SOC)/mcu1_0/$(TARGET_BUILD)/
else

  IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/mcu1_0/generated
  ifeq ($(RTOS),FREERTOS)
    LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/freertos/lib/
  else ifeq ($(RTOS),THREADX)
    LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/threadx/lib/
  endif

  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/sciserver/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/rm_pm_hal/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/sciclient_direct/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/device_manager/self_reset/lib/
  LDIRS += ${MCU_PLUS_SDK_PATH}/source/drivers/device_manager/dm_stub/lib/
  LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa/lib
endif

include $($(_MODULE)_SDIR)/../../../concerto_r5f_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_mcu1_0

ifeq ($(TARGET_OS), $(filter $(RTOS), FREERTOS THREADX))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_mcu1_0
endif

SYS_STATIC_LIBS += app_utils_hwa
SYS_STATIC_LIBS += app_utils_sciserver

ifeq ($(RTOS_SDK),pdk)
  ADDITIONAL_STATIC_LIBS += vhwa.aer5f
  ADDITIONAL_STATIC_LIBS += sciclient_direct.aer5f
  ADDITIONAL_STATIC_LIBS += sciserver_tirtos.aer5f
  ADDITIONAL_STATIC_LIBS += mailbox.aer5f
  ADDITIONAL_STATIC_LIBS += rm_pm_hal.aer5f
  ADDITIONAL_STATIC_LIBS += self_reset.aer5f
else
  ADDITIONAL_STATIC_LIBS += vhwa.am62ax.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += sciclient_direct.am62ax.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += sciserver.am62ax.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += drivers.am62ax.dm-r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += rm_pm_hal.am62ax.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += self_reset.am62ax.r5f.ti-arm-clang.${TARGET_BUILD}.lib
  ADDITIONAL_STATIC_LIBS += dm_stub.am62ax.r5f.ti-arm-clang.${TARGET_BUILD}.lib
endif
DEFS        += $(RTOS)
