
export RTOS_TYPE := $(if $(findstring FREERTOS,$(RTOS)),freertos,safertos)

SOC_UPPERCASE := $(shell echo $(SOC) | tr '[:lower:]' '[:upper:]')

SYSCFG_RTOS_DIR = $(PSDK_PATH)/platform/rtos/$(SOC)/cores
SYSCFG_CORE_SRC_DIRS = $(wildcard $(SYSCFG_RTOS_DIR)/*/src)
ARTIFACTS_OUTPUT_DIR = $(PSDK_PATH)/platform/memory_map/$(SOC)

ARTIFACTS_LINK_CORE = mcu2_0
ARTIFACTS_TARGET_DIR = $(PLATFORM_PATH)/rtos/$(SOC)/cores/$(ARTIFACTS_LINK_CORE)/src/generated

SYSCFG_DEVICE_j722s =  J722S_TDA4VEN_TDA4AEN_AM67
SYSCFG_DEVICE_j721e  = J721E_DRA829_TDA4VM_AM752x
SYSCFG_DEVICE_j721s2 = J721S2_TDA4VE_TDA4AL_TDA4VL_AM68x
SYSCFG_DEVICE_j742s2 = J742S2
SYSCFG_DEVICE_j784s4 = J784S4_TDA4AP_TDA4VP_TDA4AH_TDA4VH_AM69x
SYSCFG_DEVICE        = $(SYSCFG_DEVICE_$(SOC))

SYSCFG_GUI_PACKAGE_j722s  = AMW
SYSCFG_GUI_PACKAGE_j721e  = ALF
SYSCFG_GUI_PACKAGE_j721s2 = ALZ
SYSCFG_GUI_PACKAGE_j742s2 = AND_6
SYSCFG_GUI_PACKAGE_j784s4 = ALY
SYSCFG_GUI_PACKAGE        = $(SYSCFG_GUI_PACKAGE_$(SOC))

SOC_CORES = $($(SOC_UPPERCASE)_CORES)

ifneq ($(ECU_BUILD), $(filter $(ECU_BUILD), no))
    J722S_MCU2_0_SCRIPT = --script mcu2_0/src/$(RTOS_TYPE)_ecu_$(ECU_BUILD).syscfg --context main-r5fss0-0    --output mcu2_0/src/generated
else
    J722S_MCU2_0_SCRIPT = --script mcu2_0/src/$(RTOS_TYPE).syscfg --context main-r5fss0-0    --output mcu2_0/src/generated
endif

J722S_CORES = \
    --script mcu1_0/src/$(RTOS_TYPE).syscfg --context wkup-r5fss0-0    --output mcu1_0/src/generated \
    $(J722S_MCU2_0_SCRIPT) \
    --script c7x_1/src/$(RTOS_TYPE).syscfg  --context c75ss0-0         --output c7x_1/src/generated  \
    --script c7x_2/src/$(RTOS_TYPE).syscfg  --context c75ss1-0         --output c7x_2/src/generated

J721E_CORES = \
    --script mcu1_0/src/$(RTOS_TYPE).syscfg --context mcu-r5fss0-0 --output mcu1_0/src/generated \
    --script mcu2_0/src/$(RTOS_TYPE).syscfg --context r5fss0-0     --output mcu2_0/src/generated \
    --script mcu2_1/src/$(RTOS_TYPE).syscfg --context r5fss0-1     --output mcu2_1/src/generated \
    --script mcu3_0/src/$(RTOS_TYPE).syscfg --context r5fss1-0     --output mcu3_0/src/generated \
    --script mcu3_1/src/$(RTOS_TYPE).syscfg --context r5fss1-1     --output mcu3_1/src/generated \
    --script c6x_1/src/$(RTOS_TYPE).syscfg  --context c66ss0-0     --output c6x_1/src/generated \
    --script c6x_2/src/$(RTOS_TYPE).syscfg  --context c66ss1-0     --output c6x_2/src/generated \
    --script c7x_1/src/$(RTOS_TYPE).syscfg  --context c71ss0-0     --output c7x_1/src/generated

J721S2_CORES = \
    --script mcu1_0/src/$(RTOS_TYPE).syscfg --context mcu-r5fss0-0 --output mcu1_0/src/generated \
    --script mcu2_0/src/$(RTOS_TYPE).syscfg --context r5fss0-0     --output mcu2_0/src/generated \
    --script mcu2_1/src/$(RTOS_TYPE).syscfg --context r5fss0-1     --output mcu2_1/src/generated \
    --script mcu3_0/src/$(RTOS_TYPE).syscfg --context r5fss1-0     --output mcu3_0/src/generated \
    --script mcu3_1/src/$(RTOS_TYPE).syscfg --context r5fss1-1     --output mcu3_1/src/generated \
    --script c7x_1/src/$(RTOS_TYPE).syscfg  --context c71ss0-0     --output c7x_1/src/generated  \
    --script c7x_2/src/$(RTOS_TYPE).syscfg  --context c71ss1-0     --output c7x_2/src/generated

J742S2_CORES = \
    --script mcu1_0/src/$(RTOS_TYPE).syscfg --context mcu-r5fss0-0 --output mcu1_0/src/generated \
    --script mcu2_0/src/$(RTOS_TYPE).syscfg --context r5fss0-0     --output mcu2_0/src/generated \
    --script mcu2_1/src/$(RTOS_TYPE).syscfg --context r5fss0-1     --output mcu2_1/src/generated \
    --script mcu3_0/src/$(RTOS_TYPE).syscfg --context r5fss1-0     --output mcu3_0/src/generated \
    --script mcu3_1/src/$(RTOS_TYPE).syscfg --context r5fss1-1     --output mcu3_1/src/generated \
    --script mcu4_0/src/$(RTOS_TYPE).syscfg --context r5fss2-0     --output mcu4_0/src/generated \
    --script mcu4_1/src/$(RTOS_TYPE).syscfg --context r5fss2-1     --output mcu4_1/src/generated \
    --script c7x_1/src/$(RTOS_TYPE).syscfg  --context c71ss0-0     --output c7x_1/src/generated  \
    --script c7x_2/src/$(RTOS_TYPE).syscfg  --context c71ss1-0     --output c7x_2/src/generated  \
    --script c7x_3/src/$(RTOS_TYPE).syscfg  --context c71ss2-0     --output c7x_3/src/generated

J784S4_CORES = \
    --script mcu1_0/src/$(RTOS_TYPE).syscfg --context mcu-r5fss0-0 --output mcu1_0/src/generated \
    --script mcu2_0/src/$(RTOS_TYPE).syscfg --context r5fss0-0     --output mcu2_0/src/generated \
    --script mcu2_1/src/$(RTOS_TYPE).syscfg --context r5fss0-1     --output mcu2_1/src/generated \
    --script mcu3_0/src/$(RTOS_TYPE).syscfg --context r5fss1-0     --output mcu3_0/src/generated \
    --script mcu3_1/src/$(RTOS_TYPE).syscfg --context r5fss1-1     --output mcu3_1/src/generated \
    --script mcu4_0/src/$(RTOS_TYPE).syscfg --context r5fss2-0     --output mcu4_0/src/generated \
    --script mcu4_1/src/$(RTOS_TYPE).syscfg --context r5fss2-1     --output mcu4_1/src/generated \
    --script c7x_1/src/$(RTOS_TYPE).syscfg  --context c71ss0-0     --output c7x_1/src/generated  \
    --script c7x_2/src/$(RTOS_TYPE).syscfg  --context c71ss1-0     --output c7x_2/src/generated  \
    --script c7x_3/src/$(RTOS_TYPE).syscfg  --context c71ss2-0     --output c7x_3/src/generated  \
    --script c7x_4/src/$(RTOS_TYPE).syscfg  --context c71ss3-0     --output c7x_4/src/generated

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

platform_hlos: platform_check_ipc_cores platform_syscfg
ifeq (yes, $(filter yes, $(BUILD_LINUX_MPU) $(BUILD_QNX_MPU)))
ifeq ($(BUILD_TARGET_MODE), yes)
	$(MAKE) -C $(PLATFORM_PATH) hlos
endif
endif
ifeq ($(BUILD_EMULATION_MODE), yes)
	$(MAKE) -C $(PLATFORM_PATH) pc
endif
	$(MAKE) -C $(PLATFORM_PATH) cp_to_lib

platform_rtos: platform_check_ipc_cores platform_syscfg
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
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4 j722s))
	@for f in app_mem_map.h k3-$(SOC)-rtos-memory-map.dtsi; do rm -f $(ARTIFACTS_OUTPUT_DIR)/$$f; done
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4 j722s))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu1_0/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4 j722s))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu2_0/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu2_1/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu3_0/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu3_1/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j742s2 j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu4_0/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j742s2 j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/mcu4_1/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e))
	rm -rf $(SYSCFG_RTOS_DIR)/c6x_1/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e))
	rm -rf $(SYSCFG_RTOS_DIR)/c6x_1/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4 j722s))
	rm -rf $(SYSCFG_RTOS_DIR)/c7x_1/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721s2 j742s2 j784s4 j722s))
	rm -rf $(SYSCFG_RTOS_DIR)/c7x_2/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j742s2 j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/c7x_3/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j784s4))
	rm -rf $(SYSCFG_RTOS_DIR)/c7x_4/src/generated
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j742s2 j784s4 j722s))
	rm -rf $(PLATFORM_PATH)/memory_map/$(SOC)
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

ifeq ($(SOC),$(filter $(SOC), j722s j721e j721s2 j784s4 j742s2))
ifeq ($(SOC), j722s)
    PLATFORM_SYSCFG_PRODUCT = $(MCU_PLUS_SDK_PATH)/.metadata/product.json
else ifeq ($(SOC), j721s2)
    PLATFORM_SYSCFG_PRODUCT = $(PSDK_PATH)/platform/.metadata/product.json
else ifeq ($(SOC), j721e)
    PLATFORM_SYSCFG_PRODUCT = $(PSDK_PATH)/platform/.metadata/product.json
else ifeq ($(SOC), j742s2)
    PLATFORM_SYSCFG_PRODUCT = $(PSDK_PATH)/platform/.metadata/product.json
else
    PLATFORM_SYSCFG_PRODUCT = $(PSDK_PATH)/platform/.metadata/product.json
endif
endif

platform_syscfg_gui:
# JIRA: ADASVISION-7508 - Below if statement should be reverted once memory configurator is enabled in SPARC for TDA5 device
ifneq ($(SOC),$(filter $(SOC), tda54))
	@echo "Opening all $(SOC) cores in SysConfig GUI (system-level view)..."
	cd $(SYSCFG_RTOS_DIR) && RTOS=$(RTOS) $(SYSCFG_NWJS) --disable-gpu $(SYSCFG_PATH)/app \
		--product $(PLATFORM_SYSCFG_PRODUCT) \
		--device "$(SYSCFG_DEVICE)" \
		--part Default --package "$(SYSCFG_GUI_PACKAGE)" \
		$(SOC_CORES) > /dev/null 2>&1 &
endif

platform_syscfg:
# JIRA: ADASVISION-7508 - Below if statement should be reverted once memory configurator is enabled in SPARC for TDA5 device
ifneq ($(SOC),$(filter $(SOC), tda54))
	@echo "Generating SysConfig files for all $(SOC) cores (system-level)..."
	@for d in $(SYSCFG_CORE_SRC_DIRS); do for f in app_mem_map.h k3-$(SOC)-rtos-memory-map.dtsi; do p="$$d/generated/$$f"; if [ -L "$$p" ]; then rm -f "$$p"; fi; done; done
	cd $(SYSCFG_RTOS_DIR) && RTOS=$(RTOS) $(SYSCFG_NODE) $(SYSCFG_CLI_PATH)/dist/cli.js \
		--product $(PLATFORM_SYSCFG_PRODUCT) \
		--device "$(SYSCFG_DEVICE)" \
		$(SOC_CORES)
	@echo "Linking memory configurator artifacts into $(ARTIFACTS_OUTPUT_DIR)..."
	@mkdir -p $(ARTIFACTS_OUTPUT_DIR)
	@for f in app_mem_map.h k3-$(SOC)-rtos-memory-map.dtsi; do ln -sfn "$(ARTIFACTS_TARGET_DIR)/$$f" "$(ARTIFACTS_OUTPUT_DIR)/$$f"; done
	@echo "SysConfig generation complete for $(SOC)!"
endif

.PHONY: platform_hlos platform_rtos platform_check_ipc_cores platform_clean platform_scrub platform_syscfg platform_syscfg_gui
