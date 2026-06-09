DEFS+=CPU_rmcu2_1

ifeq ($(RTOS),FREERTOS)
    LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $($(_MODULE)_SDIR)/../concerto_r52p_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_rmcu2_1
ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
    STATIC_LIBS += app_rtos
endif

# SYS_STATIC_LIBS += app_utils_sciclient

ADDITIONAL_STATIC_LIBS += drivers.tda54.rmcu.ti-arm-clang.${TARGET_BUILD}.lib
