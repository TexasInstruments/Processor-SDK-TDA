#
# Utility makefile to build DMAUTILS libraries
#
# Edit this file to suit your specific build needs
#

# Build profile configuration
ifeq ($(PROFILE), $(filter $(PROFILE),release all))
BUILD_PROFILE_LIST_ALL+=release
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
BUILD_PROFILE_LIST_ALL+=debug
endif

# Main build target for DMAUTILS
dmautils_build:
ifeq ($(BUILD_EMULATION_MODE),yes)
	@echo "Building DMAUTILS host emulation libraries..."
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(DMA_UTILS_PATH) -f dmautils.mk dmautils-hostemu BUILD_TYPE=$(current_profile) DEVICE_NAME=$(SOC) MCUSDK_PATH=$(MCU_SDK_PATH) C76_TOOLCHAIN_PATH=$(CGT7X_ROOT) HOSTEMU_TOOLCHAIN_PATH=$(GCC_LINUX_ROOT) -s || exit 1; \
	)
endif
ifeq ($(BUILD_TARGET_MODE),yes)
	@echo "Building DMAUTILS target libraries..."
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(DMA_UTILS_PATH) -f dmautils.mk dmautils-c76 BUILD_TYPE=$(current_profile) DEVICE_NAME=$(SOC) MCUSDK_PATH=$(MCU_SDK_PATH) C76_TOOLCHAIN_PATH=$(CGT7X_ROOT) HOSTEMU_TOOLCHAIN_PATH=$(GCC_LINUX_ROOT) -s || exit 1; \
	)
endif

# Main target
dmautils: dmautils_build

# Clean build artifacts
dmautils_clean:
	@echo "Cleaning DMAUTILS build..."
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(DMA_UTILS_PATH) -f dmautils.mk dmautils-clean-$(current_profile) DEVICE_NAME=$(SOC) MCUSDK_PATH=$(MCU_SDK_PATH) C76_TOOLCHAIN_PATH=$(CGT7X_ROOT) HOSTEMU_TOOLCHAIN_PATH=$(GCC_LINUX_ROOT) -s; \
	)

# Complete clean (scrub)
dmautils_scrub: dmautils_clean
	@echo "Scrubbing DMAUTILS build directories..."
	$(MAKE) -C $(DMA_UTILS_PATH) -f dmautils.mk dmautils-clean DEVICE_NAME=$(SOC) MCUSDK_PATH=$(MCU_SDK_PATH) C76_TOOLCHAIN_PATH=$(CGT7X_ROOT) HOSTEMU_TOOLCHAIN_PATH=$(GCC_LINUX_ROOT) -s

.PHONY: dmautils dmautils_build dmautils_clean dmautils_scrub
