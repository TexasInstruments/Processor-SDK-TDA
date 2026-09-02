DEFS+=SOC_J722S

CSOURCES += generated/ti_board_config.c
CSOURCES += generated/ti_board_open_close.c
CSOURCES += generated/ti_dpl_config.c
CSOURCES += generated/ti_drivers_config.c
CSOURCES += generated/ti_drivers_open_close.c
CSOURCES += generated/ti_pinmux_config.c
CSOURCES += generated/ti_power_clock_config.c

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_$(RTOS_TYPE).cmd
LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map_$(RTOS_TYPE).cmd

ifeq ($(RTOS),SAFERTOS)
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/api/$(SAFERTOS_ISA_EXT_c7x)
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/api/NoWrapper
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/config
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/kernel/include_api
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/kernel/include_prv
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/portable/$(SAFERTOS_ISA_EXT_c7x)
  IDIRS+=$(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75/portable/$(SAFERTOS_ISA_EXT_c7x)/$(SAFERTOS_COMPILER_EXT_c7x)
endif

IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/generated

LDIRS += $(MCU_PLUS_SDK_PATH)/source/kernel/$(RTOS_TYPE)/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/drivers/lib/
LDIRS += $(MCU_PLUS_SDK_PATH)/source/board/lib/

include $($(_MODULE)_SDIR)/../../../concerto_c7x_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_c7x_1

ifeq ($(RTOS), $(filter $(RTOS), FREERTOS SAFERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_c7x_1
endif

TIDL_LIBS =
TIDL_LIBS += common_C7524
TIDL_LIBS += mmalib_C7524
TIDL_LIBS += mmalib_cn_C7524
TIDL_LIBS += perfEst_C7524
TIDL_LIBS += tidl_algo
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
  TIDL_LIBS += tidl_priv
  TIDL_LIBS += tidl_kernels
  TIDL_LIBS += tidl_ref
else
  TIDL_LIBS += tidl_priv_algo
  TIDL_LIBS += tidl_obj_algo
endif
TIDL_LIBS += tidl_custom

SYS_STATIC_LIBS += $(TIDL_LIBS)

ADDITIONAL_STATIC_LIBS += dmautils.j722s.c75ss0-0.ti-c7000.${TARGET_BUILD}.lib
ADDITIONAL_STATIC_LIBS += drivers.j722s.c75ss0-0.ti-c7000.${TARGET_BUILD}.lib

#
# Suppress this warning, 10063-D: entry-point symbol other than "_c_int00" specified
# c7x boots in secure mode and to switch to non-secure mode we need to start at a special entry point '_c_int00_secure'
# and later after switching to non-secure mode, sysbios jumps to usual entry point of _c_int00
# Hence we need to suppress this warning
CFLAGS+=--diag_suppress=10063
CFLAGS+=--diag_suppress=770
CFLAGS+=--diag_suppress=69
CFLAGS+=--diag_suppress=70
CFLAGS+=--diag_suppress=552
