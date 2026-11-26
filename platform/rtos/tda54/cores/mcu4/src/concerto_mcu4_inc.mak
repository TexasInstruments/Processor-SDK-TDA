DEFS+=CPU_mcu4
RTOS_LC := $(call lowercase,$(RTOS))

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_$(RTOS_TYPE).cmd

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

include $($(_MODULE)_SDIR)/../../../concerto_m55_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_mcu4

ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_mcu4
endif
