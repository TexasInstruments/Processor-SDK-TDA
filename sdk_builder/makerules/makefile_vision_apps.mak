#
# Utility makefile to build vision_apps libaries and related components
#
# Edit this file to suit your specific build needs
#

export RTOS_TYPE := $(if $(findstring FREERTOS,$(RTOS)),freertos,safertos)

ifeq ($(BUILD_MCU_BOARD_DEPENDENCIES),yes)
	ifneq ($(ECU_BUILD), no) # ECU build 
		MCU_SYSCFG_FILE=$(RTOS_TYPE)_ecu_$(ECU_BUILD).syscfg 
	else
		MCU_SYSCFG_FILE=$(RTOS_TYPE).syscfg
	endif
else
	MCU_SYSCFG_FILE=$(RTOS_TYPE)_no_board_deps.syscfg
endif
 
VISION_APPS_LINUX_C7x_1_IPC=$(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/vx_app_rtos_linux_c7x_1.out
VISION_APPS_LINUX_C7x_1_IPC_STRIP=$(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/vx_app_rtos_linux_c7x_1_strip.out


edgeai_deps:
ifneq (,$(filter yes,$(BUILD_LINUX_MPU) $(BUILD_QNX_MPU) $(BUILD_EMULATION_MODE)))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))
ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
	$(MAKE) -C $(VISION_APPS_PATH) tivision_apps
	$(MAKE) edgeai
endif
endif
endif

vision_apps: edgeai_deps
ifeq ($(SOC), j722s)
ifeq ($(BUILD_CPU_MCU2_0),yes)
	@echo Generating SysConfig files for mcu2_0 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context main-r5fss0-0 --part Default --package AMW --output $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/mcu2_0/generated $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/mcu2_0/$(MCU_SYSCFG_FILE)
endif
ifeq ($(BUILD_CPU_MCU1_0),yes)
	@echo Generating SysConfig files for mcu1_0 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context wkup-r5fss0-0 --part Default --package AMW --output $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/mcu1_0/generated $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/mcu1_0/$(MCU_SYSCFG_FILE)
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	@echo Generating SysConfig files for c7x_1 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context c75ss0-0 --part Default --package AMW --output $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/c7x_1/generated $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/c7x_1/$(MCU_SYSCFG_FILE)
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	@echo Generating SysConfig files for c7x_2 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context c75ss1-0 --part Default --package AMW --output $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/c7x_2/generated $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/c7x_2/$(MCU_SYSCFG_FILE)
endif
endif
ifeq ($(SOC), am62a)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	@echo Generating SysConfig files for mcu1_0 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context r5fss0-0 --part Default --package AMB --output $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/mcu1_0/generated $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/mcu1_0/example.syscfg
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	@echo Generating SysConfig files for c7x_1 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context c75ss0-0 --part Default --package AMB --output $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/c7x_1/generated $(PSDK_PATH)/vision_apps/platform/$(SOC)/rtos/c7x_1/example.syscfg
endif
endif
	$(MAKE) -C $(VISION_APPS_PATH) check_ipc_cores
	$(MAKE) -C $(VISION_APPS_PATH)
ifeq ($(SOC), am62a)
ifeq ($(BUILD_QNX_MPU),no)
ifeq ($(BUILD_CPU_C7x_1),yes)
ifeq ($(TISDK_IMAGE), edgeai)
	$(eval IMAGE_NAME := dsp_edgeai_c7x_1_release.out)
	$(eval IMAGE_NAME_STRIP := dsp_edgeai_c7x_1_release_strip.out)
else
	$(eval IMAGE_NAME := dsp_adas_c7x_1_release.out)
	$(eval IMAGE_NAME_STRIP := dsp_adas_c7x_1_release_strip.out)
endif
ifneq ($(wildcard $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME)),)
	rm -f $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME)
	rm -f $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME_STRIP)
endif
	@echo Generating binaries for c7x_1 vision_apps
	cp $(VISION_APPS_LINUX_C7x_1_IPC) $(VISION_APPS_LINUX_C7x_1_IPC_STRIP)
	$(CGT7X_ROOT)/bin/strip7x -p $(VISION_APPS_LINUX_C7x_1_IPC_STRIP)
	ln -s $(VISION_APPS_LINUX_C7x_1_IPC) $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME)
	ln -s $(VISION_APPS_LINUX_C7x_1_IPC_STRIP) $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME_STRIP)
endif
endif
endif

vision_apps_clean:
	$(MAKE) -C $(VISION_APPS_PATH) clean

vision_apps_scrub:
ifeq ($(SOC), j722s)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	rm -rf $(VISION_APPS_PATH)/platform/$(SOC)/rtos/mcu1_0/generated
endif
ifeq ($(BUILD_CPU_MCU2_0),yes)
	rm -rf $(VISION_APPS_PATH)/platform/$(SOC)/rtos/mcu2_0/generated
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	rm -rf $(VISION_APPS_PATH)/platform/$(SOC)/rtos/c7x_1/generated
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	rm -rf $(VISION_APPS_PATH)/platform/$(SOC)/rtos/c7x_2/generated
endif
endif
ifeq ($(SOC), am62a)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	rm -rf $(VISION_APPS_PATH)/platform/$(SOC)/rtos/mcu1_0/generated
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	rm -rf $(VISION_APPS_PATH)/platform/$(SOC)/rtos/c7x_1/generated
endif
endif
	$(MAKE) -C $(VISION_APPS_PATH) scrub
	$(MAKE) edgeai_scrub

vision_apps_docs:
	$(MAKE) -C $(VISION_APPS_PATH) doxy_docs

ifeq ($(SOC), j722s)
vision_apps_syscfg_gui:
	$(SYSCFG_NWJS) $(SYSCFG_PATH) --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --part Default --output $(PSDK_PATH)/vision_apps/platform/j722s/rtos/mcu2_0/generated $(PSDK_PATH)/vision_apps/platform/j722s/rtos/mcu2_0/$(MCU_SYSCFG_FILE)
endif

.PHONY: vision_apps vision_apps_clean vision_apps_scrub vision_apps_docs
