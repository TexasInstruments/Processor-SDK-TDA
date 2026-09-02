ifeq ($(RTOS),FREERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

include $($(_MODULE)_SDIR)/../../../concerto_m55_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_mcu0

ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_mcu0
endif

DEFS+= $(RTOS)
