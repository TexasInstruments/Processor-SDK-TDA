DEFS+=CPU_mcu2_0
RTOS_LC := $(call lowercase,$(RTOS))

ifeq ($(RTOS),SYSBIOS)
  XDC_BLD_FILE = $($(_MODULE)_SDIR)/../bios_cfg/config_r5f.bld
  XDC_IDIRS    = $($(_MODULE)_SDIR)/../bios_cfg/;$(BIOS_PATH)/packages/ti/posix/ccs
  XDC_CFG_FILE = $($(_MODULE)_SDIR)/mcu2_0.cfg
  XDC_PLATFORM = "ti.platforms.cortexR:J7ES_MAIN"
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker.cmd
endif
ifeq ($(RTOS),FREERTOS)
  CSOURCES += $(SOC)_mpu_cfg.c
  LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/$(SOC)_linker_freertos.cmd
endif

LINKER_CMD_FILES +=  $($(_MODULE)_SDIR)/linker_mem_map.cmd

IDIRS+=$(ETHFW_PATH)

LDIRS += $(PDK_PATH)/packages/ti/drv/ipc/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/

LDIRS += $(PDK_PATH)/packages/ti/drv/enet/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/enet/lib/$(SOC)_evm/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/enet/lib/r5f/$(TARGET_BUILD)/

LDIRS += $(PDK_PATH)/packages/ti/drv/csirx/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/csitx/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/dss/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
LDIRS += $(PDK_PATH)/packages/ti/drv/vhwa/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/

LDIRS += $(ETHFW_PATH)/out/$(TARGET_SOC)/R5Ft/$(TARGET_OS)/$(TARGET_BUILD)
LDIRS += $(REMOTE_DEVICE_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)

ifeq ($(RTOS),FREERTOS)
  LDIRS += $(PDK_PATH)/packages/ti/kernel/lib/$(SOC)/mcu2_0/$(TARGET_BUILD)/
endif

ifeq ($(RTOS), $(filter $(RTOS), FREERTOS))
  LDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/lib/$(RTOS_LC)/$(SOC)/r5f/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-stack/contrib/lib/$(RTOS_LC)/$(SOC)/r5f/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/transport/lwip/lwip-port/lib/$(RTOS_LC)/${SOC}/r5f/$(TARGET_BUILD)/
  LDIRS += $(PDK_PATH)/packages/ti/drv/enet/lib/$(RTOS_LC)/$(SOC)/r5f/$(TARGET_BUILD)/
  ifeq ($(ETHFW_GPTP_BUILD_SUPPORT),yes)
    LDIRS += $(PDK_PATH)/packages/ti/transport/tsn/lib/r5f/$(TARGET_BUILD)/
    LDIRS += $(PDK_PATH)/packages/ti/drv/mmcsd/lib/r5f/$(TARGET_BUILD)/
    LDIRS += $(PDK_PATH)/packages/ti/fs/fatfs/lib/r5f/$(TARGET_BUILD)/
  endif
endif

include $($(_MODULE)_SDIR)/../../../concerto_r5f_inc.mak

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  DEFS+=ENABLE_ETHFW
endif

# CPU instance specific libraries
STATIC_LIBS += app_rtos_common_mcu2_0
ifeq ($(RTOS), $(filter $(RTOS), FREERTOS))
  STATIC_LIBS += ipc_common
  STATIC_LIBS += rtos_mem_mcu2_0
endif
SYS_STATIC_LIBS += app_utils_hwa
SYS_STATIC_LIBS += app_utils_csi
SYS_STATIC_LIBS += app_utils_sciclient

ifeq ($(BUILD_MCU_BOARD_DEPENDENCIES),yes)
  SYS_STATIC_LIBS += app_utils_dss
endif

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  STATIC_LIBS += app_utils_ethfw
  ETHFW_LIBS += ethfw_callbacks
  ETHFW_LIBS += eth_intervlan
  ETHFW_LIBS += ethfw_board
  ETHFW_LIBS += ethfw_remotecfg_server
  ETHFW_LIBS += ethfw_common
  ETHFW_LIBS += ethfw_abstract
  ifeq ($(ETHFW_EST_DEMO_SUPPORT),yes)
    ETHFW_LIBS += ethfw_estdemo
  endif
endif

SYS_STATIC_LIBS += $(ETHFW_LIBS)
SYS_STATIC_LIBS += $(REMOTE_DEVICE_LIBS)

ADDITIONAL_STATIC_LIBS += csirx.aer5f
ADDITIONAL_STATIC_LIBS += csitx.aer5f
ADDITIONAL_STATIC_LIBS += dss.aer5f
ADDITIONAL_STATIC_LIBS += vhwa.aer5f

ifeq ($(BUILD_ENABLE_ETHFW),yes)
  ADDITIONAL_STATIC_LIBS += enetsoc.aer5f
  ADDITIONAL_STATIC_LIBS += enet.aer5f
  ADDITIONAL_STATIC_LIBS += enetphy.aer5f
  ifeq ($(ETHFW_GPTP_BUILD_SUPPORT),yes)
    ADDITIONAL_STATIC_LIBS += tsn_gptp.aer5f
    ADDITIONAL_STATIC_LIBS += tsn_uniconf.aer5f
    ADDITIONAL_STATIC_LIBS += tsn_combase.aer5f
    ADDITIONAL_STATIC_LIBS += tsn_unibase.aer5f
    ADDITIONAL_STATIC_LIBS += ti.drv.mmcsd.aer5f
    ADDITIONAL_STATIC_LIBS += ti.fs.fatfs.aer5f
  endif

  ifeq ($(RTOS), $(filter $(RTOS), FREERTOS))
    ADDITIONAL_STATIC_LIBS += lwipcontrib_$(RTOS_LC).aer5f
    ADDITIONAL_STATIC_LIBS += lwipstack_$(RTOS_LC).aer5f
    ADDITIONAL_STATIC_LIBS += lwipport_$(RTOS_LC).aer5f
    ADDITIONAL_STATIC_LIBS += lwipif_$(RTOS_LC).aer5f
    ADDITIONAL_STATIC_LIBS += lwipific_$(RTOS_LC).aer5f
    ADDITIONAL_STATIC_LIBS += enet_intercore.aer5f
    ADDITIONAL_STATIC_LIBS += enet_example_utils_$(RTOS_LC).aer5f
    ADDITIONAL_STATIC_LIBS += enet_cfgserver_$(RTOS_LC).aer5f
  endif
endif

ADDITIONAL_STATIC_LIBS += pm_lib.aer5f
ADDITIONAL_STATIC_LIBS += sciclient.aer5f

DEFS        += $(RTOS)
