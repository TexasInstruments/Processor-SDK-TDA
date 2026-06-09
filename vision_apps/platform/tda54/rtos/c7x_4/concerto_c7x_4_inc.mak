DEFS+=CPU_c7x_4

ifeq ($(RTOS),FREERTOS)
    LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

include $($(_MODULE)_SDIR)/../concerto_c7x_inc.mak

# CPU instance specific libraries
# STATIC_LIBS += vx_target_kernels_img_proc_c71
STATIC_LIBS += app_rtos_common_c7x_4

ifeq ($(RTOS),FREERTOS)
    STATIC_LIBS += app_rtos
endif

# Suppress this warning, 10063-D: entry-point symbol other than "_c_int00" specified
# c7x boots in secure mode and to switch to non-secure mode we need to start at a special entry point '_c_int00_secure'
# and later after switching to non-secure mode, sysbios jumps to usual entry point of _c_int00
# Hence we need to suppress this warning
CFLAGS+=--diag_suppress=10063
CFLAGS+=--diag_suppress=770
CFLAGS+=--diag_suppress=69
CFLAGS+=--diag_suppress=70
