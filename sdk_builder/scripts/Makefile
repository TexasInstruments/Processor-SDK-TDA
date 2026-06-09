# PDK path is always relative to the .repo directory
export PDK_INSTALL_PATH=$(realpath ../../pdk/packages)
export MCU_PLUS_SDK_INSTALL_PATH=$(realpath ../../mcu_plus_sdk)
export MCU_SDK_INSTALL_PATH=$(realpath ../../mcu_sdk)

# Set UTILS_INSTALL_DIR to cygwin /bin directory so that env.mk doesn't fail
# trying to get XDCTOOLS path for shell expansion before it exists
export UTILS_INSTALL_DIR=/bin

# Include PDK Rules.make to define various versions of components needed
ifeq ($(SOC),$(filter $(SOC), j722s am62a))
-include $(MCU_PLUS_SDK_INSTALL_PATH)/imports.mak
else ifeq ($(SOC),tda54)
-include $(MCU_SDK_INSTALL_PATH)/imports.mk
else
-include $(PDK_INSTALL_PATH)/ti/build/Rules.make
endif

# The get_component_versions rule will echo the versions of the components.
# The shell script will be responsible for piping this into a file which can
# be sourced to setup the environment for downloading the proper tool versions.
.PHONY: get_component_versions

get_component_versions:
	@echo export BIOS_VERSION=$(BIOS_VERSION)
	@echo export XDC_VERSION=$(XDC_VERSION)
	@echo export GCC_ARCH64_VERSION=$(GCC_ARCH64_VERSION)
	@echo export CGT_C6X_VERSION=$(CGT_VERSION)
	@echo export CGT_C7X_VERSION=$(CGT_C7X_VERSION)
	@echo export CGT_ARM_VERSION=$(CGT_ARM_VERSION)
	@echo export CGT_ARMLLVM_VERSION=$(CGT_ARMLLVM_VERSION)
	@echo export NDK_VERSION=$(NDK_VERSION)
	@echo export NS_VERSION=$(NS_VERSION)
	@echo export SYSCONFIG_VERSION=$(SYSCONFIG_VERSION)
	@echo export SYSCONFIG_BUILD=$(SYSCONFIG_BUILD)
