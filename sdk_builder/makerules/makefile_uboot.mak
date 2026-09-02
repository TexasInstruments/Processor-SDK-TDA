#
# Utility makefile to build u-boot with MCU1_0 image
#
# Edit this file to suit your specific build needs
#

ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
UBOOT_APP_PROFILE=debug
endif
ifeq ($(PROFILE), $(filter $(PROFILE),release all))
UBOOT_APP_PROFILE=release
endif

PLATFORM_LINUX_DM=$(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(UBOOT_APP_PROFILE)/vx_app_rtos_linux_mcu1_0.out
PLATFORM_LINUX_DM_STRIP=$(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(UBOOT_APP_PROFILE)/vx_app_rtos_linux_mcu1_0_strip.out

PLATFORM_QNX_DM=$(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(UBOOT_APP_PROFILE)/vx_app_rtos_qnx_mcu1_0.out
PLATFORM_QNX_DM_STRIP=$(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(UBOOT_APP_PROFILE)/vx_app_rtos_qnx_mcu1_0_strip.out

CROSS_COMPILE=$(GCC_LINUX_ARM_ROOT)/bin/$(CROSS_COMPILE_LINARO)

uboot_check:
ifeq ($(BUILD_TARGET_MODE),yes)
	@if [ ! -d $(PSDK_LINUX_PATH) ]; then echo 'ERROR: $(PSDK_LINUX_PATH) not found !!!'; exit 1; fi
	@if [ ! -d $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* not found !!!'; exit 1; fi
ifeq  ($(SOC), am62a)
	@if [ ! -f $(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl31.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl31.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl32.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl32.bin not found !!!'; exit 1; fi
else
	@if [ ! -f $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl31.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl31.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl32.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl32.bin not found !!!'; exit 1; fi
endif
endif

uboot_check_firmware:
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(BUILD_LINUX_MPU),yes)
	@if [ ! -f  $(PLATFORM_LINUX_DM) ]; then echo 'ERROR: $(PLATFORM_LINUX_DM) not found !!!'; exit 1; fi
endif
ifeq ($(BUILD_QNX_MPU),yes)
	@if [ ! -f $(PLATFORM_QNX_DM) ]; then echo 'ERROR: $(PLATFORM_QNX_DM) !!!'; exit 1; fi
endif
endif

uboot_clean: uboot_check
ifeq ($(BUILD_TARGET_MODE),yes)
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* clean
	rm -rf $(PSDK_LINUX_PATH)/board-support/ti-u-boot-*/$(SOC)-arm64-linux/
	rm -rf $(PSDK_LINUX_PATH)/board-support/ti-u-boot-*/$(SOC)-arm64-qnx/
endif

uboot: uboot_check uboot_check_firmware
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(BUILD_LINUX_MPU),yes)
ifeq ($(TISDK_IMAGE), edgeai)
	$(eval IMAGE_NAME := dm_edgeai_mcu1_0_release.out)
	$(eval IMAGE_NAME_STRIP := dm_edgeai_mcu1_0_release_strip.out)
else
	$(eval IMAGE_NAME := dm_adas_mcu1_0_release.out)
	$(eval IMAGE_NAME_STRIP := dm_adas_mcu1_0_release_strip.out)
endif
ifneq ($(wildcard $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/release/$(IMAGE_NAME)),)
	rm -f $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/release/$(IMAGE_NAME)
	rm -f $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/release/$(IMAGE_NAME_STRIP)
endif
	cp $(PLATFORM_LINUX_DM) $(PLATFORM_LINUX_DM_STRIP)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_LINUX_DM_STRIP)
	ln -s $(PLATFORM_LINUX_DM) $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/release/$(IMAGE_NAME)
	ln -s $(PLATFORM_LINUX_DM_STRIP) $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/release/$(IMAGE_NAME_STRIP)
ifeq  ($(SOC), am62a)
	cp $(PLATFORM_LINUX_DM_STRIP) $(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/ti-dm/am62axx/$(IMAGE_NAME_STRIP)
	$(MAKE) -j8 -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm O=$(SOC)-arm64-linux $(SOC)x_evm_a53_defconfig
endif
ifeq  ($(SOC), j722s)
	cp $(PLATFORM_LINUX_DM_STRIP) $(PSDK_LINUX_PATH)/board-support/prebuilt-images/ti-dm/j722s/ipc_echo_testb_mcu1_0_release_strip.xer5f
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm O=$(SOC)-arm64-linux -j8 $(SOC)_evm_a53_defconfig
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j784s4 j742s2))
	cp $(PLATFORM_LINUX_DM_STRIP) $(PSDK_LINUX_PATH)/board-support/prebuilt-images/ti-dm/$(SOC)/ipc_echo_testb_mcu1_0_release_strip.xer5f
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm O=$(SOC)-arm64-linux -j8 $(SOC)_evm_a72_defconfig
endif
ifeq  ($(SOC), am62a)
	$(MAKE) -j8 -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" BL31=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl31.bin TEE=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl32.bin TI_DM=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/ti-dm/am62axx/$(IMAGE_NAME_STRIP) BINMAN_INDIRS=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm O=$(SOC)-arm64-linux
else
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" BL31=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl31.bin TEE=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl32.bin BINMAN_INDIRS=$(PSDK_LINUX_PATH)/board-support/prebuilt-images O=$(SOC)-arm64-linux
endif
endif
ifeq ($(BUILD_QNX_MPU),yes)
	cp $(PLATFORM_QNX_DM) $(PLATFORM_QNX_DM_STRIP)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_QNX_DM_STRIP)
ifeq  ($(SOC), am62a)
	cp $(PLATFORM_QNX_DM_STRIP) $(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/ti-dm/am62axx/ipc_echo_testb_mcu1_0_release_strip.xer5f
	$(MAKE) -j8 -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm O=$(SOC)-arm64-qnx  $(SOC)x_evm_a53_defconfig
endif
ifeq  ($(SOC), j722s)
	cp $(PLATFORM_QNX_DM_STRIP) $(PSDK_LINUX_PATH)/board-support/prebuilt-images/ti-dm/j722s/ipc_echo_testb_mcu1_0_release_strip.xer5f
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm O=$(SOC)-arm64-qnx -j8 $(SOC)_evm_a53_defconfig
endif
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j784s4 j742s2))
	cp $(PLATFORM_QNX_DM_STRIP) $(PSDK_LINUX_PATH)/board-support/prebuilt-images/ti-dm/$(SOC)/ipc_echo_testb_mcu1_0_release_strip.xer5f
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm O=$(SOC)-arm64-qnx -j8 $(SOC)_evm_a72_defconfig
endif
ifeq  ($(SOC), am62a)
	$(MAKE) -j8 -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" BL31=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl31.bin TEE=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm/bl32.bin BINMAN_INDIRS=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/am62a-evm O=$(SOC)-arm64-qnx
else 
	$(MAKE) -C $(PSDK_LINUX_PATH)/board-support/ti-u-boot-* ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" BL31=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl31.bin TEE=$(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl32.bin BINMAN_INDIRS=$(PSDK_LINUX_PATH)/board-support/prebuilt-images O=$(SOC)-arm64-qnx
endif
endif
endif

# UBOOT_INSTALL macro for copying over u-boot binaries to directory
# $1 : linux or qnx
# $1 : destination bootfs path
define UBOOT_INSTALL =
	$(MAKE) uboot_check
	cp $(PSDK_LINUX_PATH)/board-support/ti-u-boot-*/$(SOC)-arm64-$(1)/tispl.bin $(2)/
	cp $(PSDK_LINUX_PATH)/board-support/ti-u-boot-*/$(SOC)-arm64-$(1)/u-boot.img $(2)/
endef

.PHONY: uboot_check uboot_check_firmware uboot_clean uboot
