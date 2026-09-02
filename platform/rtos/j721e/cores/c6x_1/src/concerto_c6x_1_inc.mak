ifeq ($(RTOS),SYSBIOS)
  XDC_BLD_FILE = $($(_MODULE)_SDIR)/../bios_cfg/config_c66.bld
  XDC_IDIRS    = $($(_MODULE)_SDIR)/../bios_cfg/
  XDC_CFG_FILE = $($(_MODULE)_SDIR)/c6x_1.cfg
  XDC_PLATFORM = "ti.platforms.c6x:J7ES"
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker.cmd
endif
ifeq ($(RTOS),FREERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
endif
ifeq ($(RTOS),SAFERTOS)
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_safertos.cmd
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd


ifeq ($(RTOS),SAFERTOS)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/api/$(SAFERTOS_ISA_EXT_c66)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/api/PrivWrapperStd
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/config
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/kernel/include_api
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/kernel/include_prv
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_c66)
  IDIRS+=${SAFERTOS_KERNEL_INSTALL_PATH_c66}/source_code_and_projects/SafeRTOS/portable/$(SAFERTOS_ISA_EXT_c66)/$(SAFERTOS_COMPILER_EXT_c66)
endif

ifeq ($(RTOS),FREERTOS)
  LDIRS += $(PDK_PATH)/packages/ti/kernel/lib/$(SOC)/c66xdsp_1/$(TARGET_BUILD)/
endif

ifeq ($(RTOS),SAFERTOS)
  LDIRS += $(PDK_PATH)/packages/ti/kernel/safertos/lib/$(SOC)/c66xdsp_1/$(TARGET_BUILD)/
endif

LDIRS += $(PDK_PATH)/packages/ti/drv/ipc/lib/$(SOC)/c66xdsp_1/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)/c66xdsp_1/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)/c66xdsp_1/$(TARGET_BUILD)/

include $($(_MODULE)_SDIR)/../../../concerto_c6x_inc.mak

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_c6x_1
ifeq ($(RTOS), $(filter $(RTOS), SAFERTOS FREERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_c6x_1
endif


DEFS        += $(RTOS)
