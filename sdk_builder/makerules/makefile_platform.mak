
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

PLATFORM_LINUX_C7x_1_IPC=$(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/vx_app_rtos_linux_c7x_1.out
PLATFORM_LINUX_C7x_1_IPC_STRIP=$(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/vx_app_rtos_linux_c7x_1_strip.out

platform_check_ipc_cores:
	$(MAKE) -C $(PLATFORM_PATH) check_ipc_cores

platform_hlos: platform_check_ipc_cores
ifeq (yes, $(filter yes, $(BUILD_LINUX_MPU) $(BUILD_QNX_MPU)))
ifeq ($(BUILD_TARGET_MODE), yes)
	$(MAKE) -C $(PLATFORM_PATH) hlos
endif
endif
ifeq ($(BUILD_EMULATION_MODE), yes)
	$(MAKE) -C $(PLATFORM_PATH) pc
endif
	$(MAKE) -C $(PLATFORM_PATH) cp_to_lib

platform_rtos: platform_check_ipc_cores
ifeq ($(SOC), j722s)
ifeq ($(BUILD_CPU_MCU2_0),yes)
	@echo Generating SysConfig files for mcu2_0 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context main-r5fss0-0 --part Default --package AMW --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/$(MCU_SYSCFG_FILE)
endif
ifeq ($(BUILD_CPU_MCU1_0),yes)
	@echo Generating SysConfig files for mcu1_0 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context wkup-r5fss0-0 --part Default --package AMW --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/$(MCU_SYSCFG_FILE)
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	@echo Generating SysConfig files for c7x_1 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context c75ss0-0 --part Default --package AMW --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/$(MCU_SYSCFG_FILE)
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	@echo Generating SysConfig files for c7x_2 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context c75ss1-0 --part Default --package AMW --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_2/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_2/src/$(MCU_SYSCFG_FILE)
endif
endif
ifeq ($(SOC), am62a)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	@echo Generating SysConfig files for mcu1_0 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context r5fss0-0 --part Default --package AMB --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/example.syscfg
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	@echo Generating SysConfig files for c7x_1 vision_apps
	$(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --context c75ss0-0 --part Default --package AMB --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/example.syscfg
endif
endif
	$(MAKE) -C $(PLATFORM_PATH) rtos
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
ifneq ($(wildcard $(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME)),)
	rm -f $(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME)
	rm -f $(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME_STRIP)
endif
	@echo Generating binaries for c7x_1 vision_apps
	cp $(PLATFORM_LINUX_C7x_1_IPC) $(PLATFORM_LINUX_C7x_1_IPC_STRIP)
	$(CGT7X_ROOT)/bin/strip7x -p $(PLATFORM_LINUX_C7x_1_IPC_STRIP)
	ln -s $(PLATFORM_LINUX_C7x_1_IPC) $(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME)
	ln -s $(PLATFORM_LINUX_C7x_1_IPC_STRIP) $(PLATFORM_PATH)/out/$(TARGET_SOC)/C7504/$(RTOS)/release/$(IMAGE_NAME_STRIP)
endif
endif
endif

platform_clean:
	$(MAKE) -C $(PLATFORM_PATH) clean

platform_scrub:
ifeq ($(SOC), j722s)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	rm -rf $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/generated
endif
ifeq ($(BUILD_CPU_MCU2_0),yes)
	rm -rf $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/generated
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	rm -rf $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/generated
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	rm -rf $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_2/src/generated
endif
endif
ifeq ($(SOC), am62a)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	rm -rf $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu1_0/src/generated
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	rm -rf $(PLATFORM_PATH)/rtos/$(SOC)/cores/c7x_1/src/generated
endif
endif
	$(MAKE) -C $(PLATFORM_PATH) scrub

ifeq ($(SOC), j722s)
platform_syscfg_gui:
	$(SYSCFG_NWJS) $(SYSCFG_PATH) --product $(MCU_PLUS_SDK_PATH)/.metadata/product.json --part Default --output $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/generated $(PLATFORM_PATH)/rtos/$(SOC)/cores/mcu2_0/src/$(MCU_SYSCFG_FILE)
endif

.PHONY: platform_hlos platform_rtos platform_check_ipc_cores platform_clean platform_scrub platform_syscfg_gui
