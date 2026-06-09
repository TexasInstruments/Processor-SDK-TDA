#
# Simplified makefile to build MCU_SDK libraries for TDA54
#

# Build profile configuration
ifeq ($(PROFILE), $(filter $(PROFILE),release all))
BUILD_PROFILE_LIST_ALL+=release
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
BUILD_PROFILE_LIST_ALL+=debug
endif


# Main build target for MCU SDK
mcu_sdk_build:
	$(MAKE) -C $(MCU_SDK_PATH) TI_SDK_DEVICE=tda54 gen-buildfiles -s || exit 1;
ifeq ($(BUILD_TARGET_MODE),yes)
	@echo "Building MCU_SDK target libraries for TDA54..."
	@echo "Building profile: $(BUILD_PROFILE_LIST_ALL)"

	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(MCU_SDK_PATH) $(current_profile) TI_SDK_DEVICE=tda54 -s || exit 1; \
	)
endif
ifeq ($(BUILD_EMULATION_MODE),yes)
	@echo "Building MCU_SDK host emu libraries for TDA54..."
	@echo "Building profile: $(BUILD_PROFILE_LIST_ALL)"

	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(MCU_SDK_PATH) TI_SDK_DEVICE=tda54 -s build-hostemu-gcc-linux-$(current_profile)  || exit 1; \
	)
endif

mcu_sdk_release:
ifeq ($(BUILD_TARGET_MODE),yes)
	@echo "Building MCU_SDK libraries for TDA54 in release flow..."
	@echo "Building profile: $(BUILD_PROFILE_LIST_ALL)"

	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(MCU_SDK_PATH) $(current_profile) TI_SDK_DEVICE=tda54 -s || exit 1; \
	)
	@echo "Building MCU_SDK libraries for TDA54 in release flow done..."
endif
ifeq ($(BUILD_EMULATION_MODE),yes)
	@echo "Building MCU_SDK host emu libraries for TDA54..."
	@echo "Building profile: $(BUILD_PROFILE_LIST_ALL)"

	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(MCU_SDK_PATH) TI_SDK_DEVICE=tda54 -s build-hostemu-gcc-linux-$(current_profile)  || exit 1; \
	)
	@echo "Building MCU_SDK host emu libraries for TDA54 done..."
endif

# Main target
mcu_sdk: mcu_sdk_build

# Clean build artifacts
mcu_sdk_clean:
	@echo "Cleaning MCU_SDK build..."
ifeq ($(SOC),tda54)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(MCU_SDK_PATH) clean-$(current_profile) TI_SDK_DEVICE=tda54 -s; \
	)
endif

# Complete clean (scrub)
mcu_sdk_scrub: mcu_sdk_clean
	@echo "Scrubbing MCU_SDK build directories..."
	@if [ -d "$(MCU_SDK_PATH)/build" ]; then \
		rm -rf $(MCU_SDK_PATH)/build; \
	fi

.PHONY: mcu_sdk mcu_sdk_build mcu_sdk_release mcu_sdk_clean mcu_sdk_scrub
