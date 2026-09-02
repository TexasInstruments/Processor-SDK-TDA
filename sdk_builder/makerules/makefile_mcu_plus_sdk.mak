#
# Utility makefile to build MCU_PLUS_SDK libaries and related components
#
# Edit this file to suit your specific build needs
#

ifeq ($(PROFILE), $(filter $(PROFILE),release all))
BUILD_PROFILE_LIST_ALL+=release
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
BUILD_PROFILE_LIST_ALL+=debug
endif

-include $(MCU_PLUS_SDK_PATH)/imports.mak

mcu_plus_sdk_build:
ifeq ($(SOC),am62a)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) libs DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE).r5f.ti-arm-clang PROFILE=$(current_profile) -s; \
	)
endif
ifeq ($(SOC),j722s)
ifeq ($(RTOS),SAFERTOS)
	@echo Setting up SafeRTOS for R5F and C75
	mkdir -p $(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f
	cp -r $(SAFERTOS_KERNEL_INSTALL_PATH_r5f)/source_code_and_projects/SafeRTOS/* $(MCU_PLUS_SDK_PATH)/source/kernel/safertos/r5f
	mkdir -p $(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75
	cp -r $(SAFERTOS_KERNEL_INSTALL_PATH_c7x)/source_code_and_projects/SafeRTOS/* $(MCU_PLUS_SDK_PATH)/source/kernel/safertos/c75
	curr_dir=$(shell pwd)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL), \
		cd $(MCU_PLUS_SDK_PATH) && \
		if [ -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) ]; then \
			$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) safertos_r5f.ti-arm-clang PROFILE=$(current_profile) -s && \
			$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) safertos_c75x.ti-c7000 PROFILE=$(current_profile) -s; \
		else \
			echo "Error: makefile.j722s not found in $(MCU_PLUS_SDK_PATH)"; \
			exit 1; \
		fi; \
	)
	cd $(curr_dir)
endif
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) libs DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE).main-r5fss0-0.ti-arm-clang PROFILE=$(current_profile) -s; \
	)
endif

mcu_plus_sdk: mcu_plus_sdk_emu
ifeq ($(BUILD_TARGET_MODE),yes)
	$(MAKE) mcu_plus_sdk_build
endif

mcu_plus_sdk_emu:
ifeq ($(BUILD_EMULATION_MODE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) host-emu DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	)
endif

mcu_plus_sdk_clean:
ifeq ($(SOC),am62a)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) libs-clean DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE).r5f.ti-arm-clang clean PROFILE=$(current_profile) -s; \
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) host-emu-clean DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	)
endif
ifeq ($(SOC),j722s)
ifeq ($(RTOS),SAFERTOS)
	curr_dir=$(shell pwd)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL), \
		cd $(MCU_PLUS_SDK_PATH) && \
		if [ -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) ]; then \
			$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) safertos_r5f.ti-arm-clang_clean PROFILE=$(current_profile) -s && \
			$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) safertos_c75x.ti-c7000_clean PROFILE=$(current_profile) -s; \
		else \
			echo "Error: makefile.j722s not found in $(MCU_PLUS_SDK_PATH)"; \
			exit 1; \
		fi; \
	)
	cd $(curr_dir)
endif
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) libs-clean DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE).main-r5fss0-0.ti-arm-clang clean PROFILE=$(current_profile) -s; \
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) host-emu-clean DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) PROFILE=$(current_profile) -s; \
	)
endif

mcu_plus_sdk_scrub:
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) libs-scrub DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) -s
ifeq ($(SOC),am62a)
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE).r5f.ti-arm-clang scrub PROFILE=$(current_profile) -s;
endif
ifeq ($(SOC),j722s)
ifeq ($(RTOS),SAFERTOS)
	curr_dir=$(shell pwd)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL), \
		cd $(MCU_PLUS_SDK_PATH) && \
		if [ -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) ]; then \
			$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) safertos_r5f.ti-arm-clang_scrub PROFILE=$(current_profile) -s && \
			$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) safertos_c75x.ti-c7000_scrub PROFILE=$(current_profile) -s; \
		else \
			echo "Error: makefile.j722s not found in $(MCU_PLUS_SDK_PATH)"; \
			exit 1; \
		fi; \
	)
	cd $(curr_dir)
endif
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/source/drivers/vhwa -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE).main-r5fss0-0.ti-arm-clang scrub -s
endif
	$(MAKE) -C $(MCU_PLUS_SDK_PATH) -f makefile.$(BUILD_MCU_PLUS_SDK_DEVICE) host-emu-scrub DEVICE=$(BUILD_MCU_PLUS_SDK_DEVICE) -s

.PHONY: mcu_plus_sdk mcu_plus_sdk_clean mcu_plus_sdk_build
