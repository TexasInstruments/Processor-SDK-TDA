#
# Utility makefile for SBL boot support
#
# Edit this file to suit your build needs
#

#######################################################################################
###################################[ PDK SBL ]#########################################

# Check if the RTOS SDK is set to mcu_plus_sdk or pdk
ifeq ($(RTOS_SDK),pdk)

ifeq ($(SOC),j721e)
SCICLIENT_VERSION=V1
K3_USART=0
CFG_CONSOLE_UART=0
else ifeq ($(SOC),j721s2)
SCICLIENT_VERSION=V4
K3_USART=0x8
CFG_CONSOLE_UART=0x8
else ifeq ($(SOC),$(filter $(SOC), j784s4 j742s2))
SCICLIENT_VERSION=V6
K3_USART=0x8
CFG_CONSOLE_UART=0x8
endif

#OSPI DATA
OSPI_LOCATION_SBL_IMAGE=0
OSPI_LOCATION_SYSFW_BIN=80000
OSPI_LOCATION_BOOT_APP=100000
OSPI_LOCATION_MULTI_CORE_IMAGE_1=1FC0000
OSPI_LOCATION_MULTI_CORE_IMAGE_2=27C0000
OSPI_LOCATION_ATF_IMAGE=1C0000
OSPI_LOCATION_LINUX_DTB=1EC0000
OSPI_LOCATION_HLOS_KERNEL_IMAGE=7C0000
OSPI_PATTERN=3FE0000
OSPI_PATTERN_FILE=$(PDK_PATH)/packages/ti/board/src/flash/nor/ospi/nor_spi_patterns.bin

SBL_BOOTFILES_PATH=$(PLATFORM_PATH)/out/sbl_bootfiles

#UNIFLASH INFO
UNIFLASH_VERSION=uniflash_6.0.0
UNIFLASH_DIR=${HOME}/ti/$(UNIFLASH_VERSION)
UNIFLASH_COM_PORT=/dev/ttyUSB5
UNIFLASH_SCRIPT=$(UNIFLASH_DIR)/dslite.sh

ifneq ("$(wildcard $(UNIFLASH_SCRIPT))","")
	UNIFLASH_FOUND=yes
else
	UNIFLASH_FOUND=no
endif

#SBL INFO
SBL_CORE=mcu1_0
BOARD=$(BUILD_PDK_BOARD)
ATF_OPTEE_PATH=$(SBL_BOOTFILES_PATH)/atf_optee_dir
ATF_TARGET_BOARD=generic
ifeq ($(SOC),j784s4)
	ATF_TARGET_BOARD=j784s4
endif
SBL_REPO_PATH=$(PDK_PATH)/packages/ti/boot/sbl
MULTICORE_APPIMAGE_GEN_TOOL_PATH=$(SBL_REPO_PATH)/tools/multicoreImageGen/bin
SBL_OUT2RPRC_GEN_TOOL_PATH=$(SBL_REPO_PATH)/tools/out2rprc/bin
DEV_ID=55

REMOTE_CORE_LIST_LATEAPP1=
REMOTE_CORE_LIST_LATEAPP2=

CERT_SCRIPT=$(PDK_PATH)/packages/ti/build/makerules/x509CertificateGen.sh

CROSS_COMPILE=$(GCC_LINUX_ARM_ROOT)/bin/$(CROSS_COMPILE_LINARO)

ifeq ($(BUILD_CPU_MCU2_0),yes)
	REMOTE_CORE_LIST_LATEAPP1+=10 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu2_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU2_1),yes)
	REMOTE_CORE_LIST_LATEAPP1+=11 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu2_1.out.rprc
endif
ifeq ($(BUILD_CPU_MCU3_0),yes)
	REMOTE_CORE_LIST_LATEAPP2+=12 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu3_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU3_1),yes)
	REMOTE_CORE_LIST_LATEAPP2+=13 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu3_1.out.rprc
endif
ifeq ($(BUILD_CPU_MCU4_0),yes)
	REMOTE_CORE_LIST_LATEAPP2+=14 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu4_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU4_1),yes)
	REMOTE_CORE_LIST_LATEAPP2+=15 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu4_1.out.rprc
endif
ifeq ($(BUILD_CPU_C6x_1),yes)
	REMOTE_CORE_LIST_LATEAPP2+=16 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c6x_1.out.rprc
endif
ifeq ($(BUILD_CPU_C6x_2),yes)
	REMOTE_CORE_LIST_LATEAPP2+=17 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c6x_2.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	REMOTE_CORE_LIST_LATEAPP2+=18 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_1.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	REMOTE_CORE_LIST_LATEAPP2+=19 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_2.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_3),yes)
	REMOTE_CORE_LIST_LATEAPP2+=20 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_3.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_4),yes)
	REMOTE_CORE_LIST_LATEAPP2+=21 $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_4.out.rprc
endif

USE_OPTEE ?= 1

ifeq ($(BUILD_QNX_MPU), yes)
USE_OPTEE=0
endif

ifeq ($(BUILD_LINUX_MPU), yes)
	SBL_SD_FS_PATH=$(LINUX_SD_FS_BOOT_PATH)
	LDS_PATH=$(PLATFORM_PATH)/hlos/src/linux_lds
	# With or without OP-TEE (1 or 0)
endif
ifeq ($(BUILD_QNX_MPU), yes)
	SBL_SD_FS_PATH=$(QNX_SD_FS_BOOT_PATH)
	LDS_PATH=$(PLATFORM_PATH)/hlos/src/qnx_lds
	# With or without OP-TEE (1 or 0)
	USE_OPTEE = 0
endif

# Supported : pdk, mcusw
BOOTAPP ?= pdk

sbl_atf_optee:
ifeq ($(BUILD_QNX_MPU), yes)
ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
	# For ATF, setting HANDLE_EA_EL3_FIRST_NS=0 for QNX so that the all runtime exception to be routed to current exception level (or in EL1 if the current exception level is EL0)
	$(MAKE) -C $(PLATFORM_PATH)/../trusted-firmware-a -s -j32 CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" PLAT=k3 TARGET_BOARD=$(ATF_TARGET_BOARD) SPD=opteed  HANDLE_EA_EL3_FIRST_NS=0 K3_USART=$(K3_USART)
else
	# For ATF, setting HANDLE_EA_EL3_FIRST_NS=0 for QNX so that the all runtime exception to be routed to current exception level (or in EL1 if the current exception level is EL0)
	$(MAKE) -C $(PLATFORM_PATH)/../trusted-firmware-a -s -j32 CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" PLAT=k3 TARGET_BOARD=$(ATF_TARGET_BOARD) HANDLE_EA_EL3_FIRST_NS=0 K3_USART=$(K3_USART)
endif
endif
ifeq ($(BUILD_LINUX_MPU), yes)
ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
	$(MAKE) -C $(PLATFORM_PATH)/../trusted-firmware-a -s -j32 CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" PLAT=k3 TARGET_BOARD=$(ATF_TARGET_BOARD) SPD=opteed K3_USART=$(K3_USART)
else
	$(MAKE) -C $(PLATFORM_PATH)/../trusted-firmware-a -s -j32 CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" PLAT=k3 TARGET_BOARD=$(ATF_TARGET_BOARD) K3_USART=$(K3_USART)
endif
endif

ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
	$(MAKE) -C $(PLATFORM_PATH)/../optee_os -s -j32 CROSS_COMPILE_core=$(GCC_LINUX_ARM_ROOT)/bin/aarch64-none-linux-gnu- CROSS_COMPILE_ta_arm32=$(GCC_LINUX_ARM_ROOT)/bin/aarch64-none-linux-gnu- CROSS_COMPILE_ta_arm64=$(GCC_LINUX_ARM_ROOT)/bin/aarch64-none-linux-gnu- NOWERROR=1 CFG_TEE_TA_LOG_LEVEL=0 CFG_TEE_CORE_LOG_LEVEL=2 CFG_ARM64_core=y ta-targets=ta_arm64 PLATFORM=k3 PLATFORM_FLAVOR=j7 CFG_CONSOLE_UART=$(CFG_CONSOLE_UART)
endif
	mkdir -p $(ATF_OPTEE_PATH)
	cp $(PLATFORM_PATH)/../trusted-firmware-a/build/k3/$(ATF_TARGET_BOARD)/release/bl31.bin $(ATF_OPTEE_PATH)/bl31.bin
ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
	cp $(PLATFORM_PATH)/../optee_os/out/arm-plat-k3/core/tee-pager_v2.bin $(ATF_OPTEE_PATH)/bl32.bin
endif


sbl_atf_optee_scrub:
	$(MAKE) -C $(PLATFORM_PATH)/../trusted-firmware-a clean
	$(MAKE) -C $(PLATFORM_PATH)/../optee_os clean CFG_ARM64_core=y PLATFORM=k3 PLATFORM_FLAVOR=j7
	rm -rf $(PLATFORM_PATH)/../trusted-firmware-a/build/k3
	rm -rf $(PLATFORM_PATH)/../optee_os/out/arm-plat-k3

sbl_pdk_sd:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_mmcsd_img TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)/mmcsd/bin/sbl_mmcsd_img_$(SBL_CORE)_release.tiimage $(SBL_BOOTFILES_PATH)/tiboot3.bin
	cp $(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs.bin $(SBL_BOOTFILES_PATH)/tifs.bin

sbl_pdk_sd_hs:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_mmcsd_img_hs TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)_hs/mmcsd/bin/sbl_mmcsd_img_$(SBL_CORE)_release.tiimage $(SBL_BOOTFILES_PATH)/tiboot3.bin.signed
ifeq ($(HS_SR), 2_0)
	cp $(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs_sr2-hs-enc.bin $(SBL_BOOTFILES_PATH)/tifs.bin.signed
else ifeq ($(HS_SR), 1_1)
	cp $(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs_sr1.1-hs-enc.bin $(SBL_BOOTFILES_PATH)/tifs.bin.signed
else
	cp $(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs-hs-enc.bin $(SBL_BOOTFILES_PATH)/tifs.bin.signed
endif

sbl_pdk_ospi:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_cust_img TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)/cust/bin/sbl_cust_img_$(SBL_CORE)_release.tiimage $(SBL_BOOTFILES_PATH)/

sbl_pdk_ospi_hs:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_cust_img_hs TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)_hs/cust/bin/sbl_cust_img_$(SBL_CORE)_release.tiimage $(SBL_BOOTFILES_PATH)/sbl_cust_img_$(SBL_CORE)_release.tiimage.signed

#################
## MCU BootApp
#################
sbl_mcusw_bootimage_touch:
	touch $(MCUSW_PATH)/mcuss_demos/boot_app_mcu_rtos/boot.c
	touch $(MCUSW_PATH)/mcuss_demos/boot_app_mcu_rtos/main_tirtos.c
	touch $(MCUSW_PATH)/mcuss_demos/boot_app_mcu_rtos/soc/$(SOC)/boot_core_defs.c

sbl_mcusw_bootimage_sd:
	$(MAKE) sbl_mcusw_bootimage_touch
ifeq ($(BUILD_QNX_MPU), yes)
	$(MAKE) -C $(MCUSW_PATH)/build can_boot_app_mcu_rtos -s HLOSBOOT=qnx BOOTMODE=mmcsd BOARD=$(BOARD) CORE=$(SBL_CORE) BUILD_OS_TYPE=freertos CANFUNC=none
endif
ifeq ($(BUILD_LINUX_MPU), yes)
	$(MAKE) -C $(MCUSW_PATH)/build can_boot_app_mcu_rtos -s HLOSBOOT=linux BOOTMODE=mmcsd BOARD=$(BOARD) CORE=$(SBL_CORE) BUILD_OS_TYPE=freertos CANFUNC=none
endif
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(MCUSW_PATH)/binary/can_boot_app_mcu_rtos/bin/$(BOARD)/can_boot_app_mcu_rtos_$(SBL_CORE)_release.appimage $(SBL_BOOTFILES_PATH)/app

sbl_mcusw_bootimage_sd_hs:
	$(MAKE) sbl_mcusw_bootimage_sd
	$(CERT_SCRIPT)  -b $(SBL_BOOTFILES_PATH)/app -o $(SBL_BOOTFILES_PATH)/app.signed -c R5 -l 0x0 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem

sbl_mcusw_bootimage_ospi:
	$(MAKE) sbl_mcusw_bootimage_touch
ifeq ($(BUILD_QNX_MPU), yes)
	$(MAKE) -C $(MCUSW_PATH)/build can_boot_app_mcu_rtos -s HLOSBOOT=qnx BOOTMODE=ospi BOARD=$(BOARD) CORE=$(SBL_CORE) BUILD_OS_TYPE=freertos CANFUNC=none
endif
ifeq ($(BUILD_LINUX_MPU), yes)
	$(MAKE) -C $(MCUSW_PATH)/build can_boot_app_mcu_rtos -s HLOSBOOT=linux BOOTMODE=ospi BOARD=$(BOARD) CORE=$(SBL_CORE) BUILD_OS_TYPE=freertos CANFUNC=none
endif
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(MCUSW_PATH)/binary/can_boot_app_mcu_rtos/bin/$(BOARD)/can_boot_app_mcu_rtos_$(SBL_CORE)_release.appimage $(SBL_BOOTFILES_PATH)/can_boot_app_mcu_rtos_$(SBL_CORE)_release_ospi.appimage

sbl_mcusw_bootimage_ospi_hs:
	$(MAKE) sbl_mcusw_bootimage_ospi
	$(CERT_SCRIPT)  -b $(SBL_BOOTFILES_PATH)/can_boot_app_mcu_rtos_$(SBL_CORE)_release_ospi.appimage -o $(SBL_BOOTFILES_PATH)/can_boot_app_mcu_rtos_$(SBL_CORE)_release_ospi.appimage.signed -c R5 -l 0x0 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem


#################
## PDK BootApp
#################
pdk_bootapp_sd:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_qnx TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no -s BOARD=$(BOARD) CORE=$(SBL_CORE)
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/mmcsd/sbl_boot_app_mmcsd_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage $(SBL_BOOTFILES_PATH)/app

pdk_bootapp_sd_hs: pdk_bootapp_sd
	$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_qnx_hs TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no -s BOARD=$(BOARD) CORE=$(SBL_CORE)
	cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)_hs/mmcsd/sbl_boot_app_mmcsd_qnx_hs_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage.signed $(SBL_BOOTFILES_PATH)/app.signed

pdk_bootapp_ospi: pdk_bootapp_sd
	$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_ospi_qnx TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no -s BOARD=$(BOARD) CORE=$(SBL_CORE)
	mkdir -p $(SBL_BOOTFILES_PATH)
	cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/ospi/sbl_boot_app_ospi_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage $(SBL_BOOTFILES_PATH)/app_ospi

pdk_bootapp_ospi_hs: pdk_bootapp_ospi
	$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_ospi_qnx_hs TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no -s BOARD=$(BOARD) CORE=$(SBL_CORE)
	cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)_hs/ospi/sbl_boot_app_ospi_qnx_hs_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage.signed $(SBL_BOOTFILES_PATH)/app_ospi.signed


sbl_vision_apps_bootimage_1:
	mkdir -p $(SBL_BOOTFILES_PATH)/rprcs
ifeq ($(BUILD_CPU_MCU2_0),yes)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu2_0.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu2_0.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu2_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU2_1),yes)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu2_1.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu2_1.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu2_1.out.rprc
endif
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_BOOTFILES_PATH)/lateapp1 $(REMOTE_CORE_LIST_LATEAPP1)

sbl_vision_apps_bootimage_hs_1:
	$(MAKE) sbl_vision_apps_bootimage_1
	$(CERT_SCRIPT) -b $(SBL_BOOTFILES_PATH)/lateapp1 -o $(SBL_BOOTFILES_PATH)/lateapp1.signed -c R5 -l 0x0 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem

sbl_vision_apps_bootimage_2:
	mkdir -p $(SBL_BOOTFILES_PATH)/rprcs
ifeq ($(BUILD_CPU_MCU3_0),yes)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu3_0.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu3_0.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu3_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU3_1),yes)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu3_1.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu3_1.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu3_1.out.rprc
endif
ifeq ($(BUILD_CPU_MCU4_0),yes)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu4_0.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu4_0.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu4_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU4_1),yes)
	$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu4_1.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_mcu4_1.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_mcu4_1.out.rprc
endif
ifeq ($(BUILD_CPU_C6x_1),yes)
	$(CGT6X_ROOT)/bin/strip6x -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c6x_1.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c6x_1.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c6x_1.out.rprc
endif
ifeq ($(BUILD_CPU_C6x_2),yes)
	$(CGT6X_ROOT)/bin/strip6x -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c6x_2.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c6x_2.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c6x_2.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	$(CGT7X_ROOT)/bin/strip7x -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_1.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_1.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_1.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	$(CGT7X_ROOT)/bin/strip7x -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_2.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_2.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_2.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_3),yes)
	$(CGT7X_ROOT)/bin/strip7x -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_3.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_3.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_3.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_4),yes)
	$(CGT7X_ROOT)/bin/strip7x -p $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_4.out
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(PLATFORM_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(QNX_APP_PROFILE)/vx_app_rtos_qnx_c7x_4.out $(SBL_BOOTFILES_PATH)/rprcs/vx_app_rtos_qnx_c7x_4.out.rprc
endif
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_BOOTFILES_PATH)/lateapp2 $(REMOTE_CORE_LIST_LATEAPP2)

sbl_vision_apps_bootimage_hs_2:
	$(MAKE) sbl_vision_apps_bootimage_2
	$(CERT_SCRIPT) -b $(SBL_BOOTFILES_PATH)/lateapp2 -o $(SBL_BOOTFILES_PATH)/lateapp2.signed -c R5 -l 0x0 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem

sbl_vision_apps_bootimage: sbl_vision_apps_bootimage_1 sbl_vision_apps_bootimage_2

sbl_vision_apps_bootimage_hs:sbl_vision_apps_bootimage_hs_1 sbl_vision_apps_bootimage_hs_2

sbl_qnx_bootimage:
ifeq ($(BUILD_QNX_MPU), yes)
ifeq ("$(wildcard $(QNX_BOOT_PATH)/qnx-ifs)","")
	$(error qnx-ifs is still not built!)
endif
	mkdir -p $(SBL_BOOTFILES_PATH)/rprcs
	curr_dir=$(PWD)
ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
	cd $(ATF_OPTEE_PATH) && \
	$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/atf_optee.lds -o $(SBL_BOOTFILES_PATH)/rprcs/atf_optee.elf && \
	cd $(QNX_BOOT_PATH) && \
	$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/ifs_qnx.lds -o $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.elf && \
	cd $(curr_dir)
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_BOOTFILES_PATH)/rprcs/atf_optee.elf $(SBL_BOOTFILES_PATH)/rprcs/atf_optee.rprc
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.elf $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_BOOTFILES_PATH)/atf_optee.appimage 0 $(SBL_BOOTFILES_PATH)/rprcs/atf_optee.rprc
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage 0 $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
else
	cd $(ATF_OPTEE_PATH) && \
	$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/atf_only.lds -o $(SBL_BOOTFILES_PATH)/rprcs/atf_only.elf && \
	cd $(QNX_BOOT_PATH) && \
	$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/ifs_qnx.lds -o $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.elf && \
	cd $(curr_dir)
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_BOOTFILES_PATH)/rprcs/atf_only.elf $(SBL_BOOTFILES_PATH)/rprcs/atf_only.rprc
	mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.elf $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_BOOTFILES_PATH)/atf_optee.appimage 0 $(SBL_BOOTFILES_PATH)/rprcs/atf_only.rprc
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage 0 $(SBL_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
endif
endif

sbl_qnx_bootimage_hs:
ifeq ($(BUILD_QNX_MPU), yes)
	$(MAKE) sbl_qnx_bootimage
	$(CERT_SCRIPT) -b $(SBL_BOOTFILES_PATH)/atf_optee.appimage -o $(SBL_BOOTFILES_PATH)/atf_optee.appimage.signed -c R5 -l 0x0 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem
	$(CERT_SCRIPT) -b $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage -o $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage.signed -c R5 -l 0x0 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem
endif

ifeq ($(BOOTAPP),$(filter $(BOOTAPP), pdk))
bootapp_sd: pdk_bootapp_sd
bootapp_sd_hs: pdk_bootapp_sd_hs
bootapp_ospi: pdk_bootapp_ospi
bootapp_ospi_hs: pdk_bootapp_ospi_hs
endif
ifeq ($(BOOTAPP),$(filter $(BOOTAPP), mcusw))
bootapp_sd: sbl_mcusw_bootimage_sd
bootapp_sd_hs: sbl_mcusw_bootimage_sd_hs
bootapp_ospi: sbl_mcusw_bootimage_ospi
bootapp_ospi_hs: sbl_mcusw_bootimage_ospi_hs
endif

sbl_linux_bootimage:

sbl_bootimage_sd: sbl_pdk_sd bootapp_sd
sbl_bootimage_ospi: sbl_pdk_ospi bootapp_ospi

sbl_bootimage_sd_hs: sbl_pdk_sd_hs bootapp_sd_hs
sbl_bootimage_ospi_hs: sbl_pdk_ospi_hs bootapp_ospi_hs

sbl_bootimage: sbl_bootimage_sd sbl_bootimage_ospi sbl_atf_optee sbl_vision_apps_bootimage sbl_qnx_bootimage sbl_linux_bootimage

sbl_bootimage_hs: sbl_bootimage_sd_hs sbl_bootimage_ospi_hs sbl_atf_optee sbl_vision_apps_bootimage_hs sbl_qnx_bootimage_hs

sbl_bootimage_flash_uniflash_programmer:
ifeq ($(UNIFLASH_FOUND), yes)
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(UNIFLASH_DIR)/processors/FlashWriter/$(BOARD)/uart_$(SOC)_evm_flash_programmer_release.tiimage -i 0 || true
else
	echo "Uniflash not found! Please install uniflash or update the uniflash path in makefile"
endif

sbl_bootimage_flash_sbl_sysfw_bootapp:
ifeq ($(UNIFLASH_FOUND), yes)
	#SBL Image
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/sbl_cust_img_mcu1_0_release.tiimage -d 3 -o $(OSPI_LOCATION_SBL_IMAGE) || true
	#OSPI Patten file
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(OSPI_PATTERN_FILE) -d 3 -o $(OSPI_PATTERN) || true
	#SYSFW BIN
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/tifs.bin -d 3 -o $(OSPI_LOCATION_SYSFW_BIN) || true
	#Boot App
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/can_boot_app_mcu_rtos_mcu1_0_release_ospi.appimage -d 3 -o $(OSPI_LOCATION_BOOT_APP) || true
else
	echo "Uniflash not found! Please install uniflash or update the uniflash path in makefile"
endif

sbl_bootimage_flash_hlos:
ifeq ($(UNIFLASH_FOUND), yes)
	#ATF Image
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/atf_optee.appimage -d 3 -o $(OSPI_LOCATION_ATF_IMAGE) || true
	#HLOS Image
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage -d 3 -o $(OSPI_LOCATION_HLOS_KERNEL_IMAGE) || true
else
	echo "Uniflash not found! Please install uniflash or update the uniflash path in makefile"
endif

sbl_bootimage_flash_rtosapp:
ifeq ($(UNIFLASH_FOUND), yes)
	#Multicore Image 1
	cp -fv $(SBL_BOOTFILES_PATH)/lateapp1 $(SBL_BOOTFILES_PATH)/lateapp1.appimage
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/lateapp1.appimage -d 3 -o $(OSPI_LOCATION_MULTI_CORE_IMAGE_1) || true
	rm -rf $(SBL_BOOTFILES_PATH)/lateapp1.appimage
	#Multicore Image 2
	cp -fv $(SBL_BOOTFILES_PATH)/lateapp2 $(SBL_BOOTFILES_PATH)/lateapp2.appimage
	$(UNIFLASH_SCRIPT) --mode processors -c $(UNIFLASH_COM_PORT) -f $(SBL_BOOTFILES_PATH)/lateapp2.appimage -d 3 -o $(OSPI_LOCATION_MULTI_CORE_IMAGE_2) || true
	rm -rf $(SBL_BOOTFILES_PATH)/lateapp2.appimage
else
	echo "Uniflash not found! Please install uniflash or update the uniflash path in makefile"
endif

sbl_bootimage_install_sd: sbl_vision_apps_bootimage sbl_qnx_bootimage sbl_linux_bootimage
	cp $(SBL_BOOTFILES_PATH)/tiboot3.bin $(SBL_SD_FS_PATH)
	cp $(SBL_BOOTFILES_PATH)/tifs.bin $(SBL_SD_FS_PATH)
	cp $(SBL_BOOTFILES_PATH)/app $(SBL_SD_FS_PATH)
	cp $(SBL_BOOTFILES_PATH)/lateapp* $(SBL_SD_FS_PATH)
ifeq ($(BUILD_QNX_MPU), yes)
	cp $(SBL_BOOTFILES_PATH)/atf_optee.appimage $(SBL_SD_FS_PATH)
	cp $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage $(SBL_SD_FS_PATH)
endif
	sync

sbl_bootimage_hs_install_sd: sbl_vision_apps_bootimage_hs sbl_qnx_bootimage_hs
	cp $(SBL_BOOTFILES_PATH)/tiboot3.bin.signed $(SBL_SD_FS_PATH)/tiboot3.bin
	cp $(SBL_BOOTFILES_PATH)/tifs.bin.signed $(SBL_SD_FS_PATH)/tifs.bin
	cp $(SBL_BOOTFILES_PATH)/app.signed $(SBL_SD_FS_PATH)/app
	cp $(SBL_BOOTFILES_PATH)/lateapp1.signed $(SBL_SD_FS_PATH)/lateapp1
	cp $(SBL_BOOTFILES_PATH)/lateapp2.signed $(SBL_SD_FS_PATH)/lateapp2
ifeq ($(BUILD_QNX_MPU), yes)
	cp $(SBL_BOOTFILES_PATH)/atf_optee.appimage.signed $(SBL_SD_FS_PATH)/atf_optee.appimage
	cp $(SBL_BOOTFILES_PATH)/ifs_qnx.appimage.signed $(SBL_SD_FS_PATH)/ifs_qnx.appimage
endif
	sync

sbl_bootimage_install_ospi: sbl_vision_apps_bootimage sbl_qnx_bootimage sbl_linux_bootimage
	$(MAKE) sbl_bootimage_flash_uniflash_programmer
	$(MAKE) sbl_bootimage_flash_sbl_sysfw_bootapp
	$(MAKE) sbl_bootimage_flash_rtosapp
	$(MAKE) sbl_bootimage_flash_hlos

sbl_bootimage_scrub: sbl_atf_optee_scrub
	rm -rf $(PDK_PATH)/packages/ti/binary/sbl_*
	rm -rf $(PDK_PATH)/packages/ti/binary/ti/boot/
	rm -rf $(PDK_PATH)/packages/ti/boot/sbl/binary
	rm -rf $(MCUSW_PATH)/binary
	rm -rf $(SBL_BOOTFILES_PATH)/

sbl_bootimage_clean: sbl_bootimage_scrub

endif # ifeq ($(RTOS_SDK),pdk)