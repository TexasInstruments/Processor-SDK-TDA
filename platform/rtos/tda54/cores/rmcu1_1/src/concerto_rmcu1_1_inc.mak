DEFS+=CPU_rmcu1_1

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_$(RTOS_TYPE).cmd

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

include $($(_MODULE)_SDIR)/../../../concerto_r52p_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_rmcu1_1

ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_rmcu1_1
endif