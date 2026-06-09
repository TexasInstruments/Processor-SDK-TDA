#
# Utility makefile for SBL boot support with combined appimage tool
#
# Edit this file to suit your build needs
#

#######################################################################################
###################################[ PDK SBL ]#########################################

# To build SBL combined appimage for LINUX/QNX MPU, run the following commands:
# make sbl_combined_bootimage   -> Generate all the required files (tiboot3.bin, tifs.bin, app, lateapp1, lateapp2, ifs_qnx.appimage/atf_optee.appimage for QNX or atf_optee.appimage, tidtb_linux.appimage and tikernelimage_linux.appimage for Linux)
#
# or use below individual targets:
#
# make sbl_sd_hlos              -> tiboot3.bin, tifs.bin
# make sbl_ospi_hlos            -> tiboot3.bin, tifs.bin
# make sbl_bootapp              -> app
# make sbl_lateapps             -> lateapp1, lateapp2
# make sbl_appimage             -> ifs_qnx.appimage, atf_optee.appimage (for QNX) / atf_optee.appimage, tidtb_linux.appimage and tikernelimage_linux.appimage (for Linux)

# make sbl_combined_bootimage_install_sd -> to copy all the generated files to the SD card LINUX/QNX boot path


# Check if the RTOS SDK is set to mcu_plus_sdk or pdk
ifeq ($(RTOS_SDK),pdk)

SBL_CORE=mcu1_0
BOARD=$(BUILD_PDK_BOARD)

SBL_COMBINED_BOOTFILES_PATH=$(VISION_APPS_PATH)/out/sbl_combined_bootfiles
COMBINED_APPIMAGE_TOOL_PATH=$(SBL_REPO_PATH)/tools/combined_appimage
SBL_REPO_PATH=$(PDK_PATH)/packages/ti/boot/sbl
CROSS_COMPILE=$(GCC_LINUX_ARM_ROOT)/bin/$(CROSS_COMPILE_LINARO)
MULTICORE_APPIMAGE_GEN_TOOL_PATH=$(SBL_REPO_PATH)/tools/multicoreImageGen/bin
SBL_OUT2RPRC_GEN_TOOL_PATH=$(SBL_REPO_PATH)/tools/out2rprc/bin
CERT_SCRIPT=$(PDK_PATH)/packages/ti/build/makerules/x509CertificateGen.sh
LINUX_BUILD_DIR_PATH=${SBL_REPO_PATH}/tools/combined_appimage/bin/${BOARD}/

ATF_TARGET_BOARD=generic
ifeq ($(SOC),j784s4)
	ATF_TARGET_BOARD=j784s4
endif

# For hs-fs board, SOC_TYPE will be hs_fs. For hs-se board, SOC_TYPE will be hs
SOC_TYPE?=gp

# Setting OS type (like freertos, safertos) here in lowercase letter
BUILD_OS_TYPE?=$(shell echo $(RTOS) | tr '[:upper:]' '[:lower:]')

# Setting HLOS and APP_PROFILE variable based on build type
ifeq ($(BUILD_QNX_MPU), yes)
HLOS := qnx
else ifeq ($(BUILD_LINUX_MPU), yes)
HLOS := linux
endif

ifeq ($(HLOS),qnx)
APP_PROFILE := $(QNX_APP_PROFILE)
else ifeq ($(HLOS),linux)
APP_PROFILE := $(LINUX_APP_PROFILE)
endif

ifeq ($(SOC_TYPE),gp)
LATEAPP1_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/lateapp1
LATEAPP2_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/lateapp2
ATF_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage
DTB_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/tidtb_linux.appimage
KERNEL_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/tikernelimage_linux.appimage
TIFS_BINARY_PATH=$(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs.bin
BOOTAPP_IMAGE_PATH=$(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/mmcsd/sbl_boot_app_mmcsd_$(HLOS)_$(BOARD)_$(SBL_CORE)_$(BUILD_OS_TYPE)_TestApp_release.appimage
else ifeq ($(SOC_TYPE),hs_fs)
LATEAPP1_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/lateapp1.hs_fs
LATEAPP2_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/lateapp2.hs_fs
ATF_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage.hs_fs
DTB_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/tidtb_linux.appimage.hs_fs
KERNEL_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/tikernelimage_linux.appimage.hs_fs
TIFS_BINARY_PATH=$(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs-hs-fs-enc.bin
BOOTAPP_IMAGE_PATH=$(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/mmcsd/sbl_boot_app_mmcsd_$(HLOS)_$(BOARD)_$(SBL_CORE)_$(BUILD_OS_TYPE)_TestApp_release.appimage.hs_fs
else ifeq ($(SOC_TYPE),hs)
LATEAPP1_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/lateapp1.signed
LATEAPP2_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/lateapp2.signed
ATF_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage.signed
DTB_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/tidtb_linux.appimage.signed
KERNEL_BINARY_PATH=$(SBL_COMBINED_BOOTFILES_PATH)/tikernelimage_linux.appimage.signed
TIFS_BINARY_PATH=$(PDK_PATH)/packages/ti/drv/sciclient/soc/$(SCICLIENT_VERSION)/tifs-hs-enc.bin
BOOTAPP_IMAGE_PATH=$(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/mmcsd/sbl_boot_app_mmcsd_$(HLOS)_$(BOARD)_$(SBL_CORE)_$(BUILD_OS_TYPE)_TestApp_release.appimage.signed
endif

REMOTE_CORE_LIST_LATEAPP1_COMBINED=
REMOTE_CORE_LIST_LATEAPP2_COMBINED=

ifeq ($(BUILD_CPU_MCU2_0),yes)
    REMOTE_CORE_LIST_LATEAPP1_COMBINED+=10 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu2_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU2_1),yes)
    REMOTE_CORE_LIST_LATEAPP1_COMBINED+=11 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu2_1.out.rprc
endif
ifeq ($(BUILD_CPU_MCU3_0),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=12 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu3_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU3_1),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=13 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu3_1.out.rprc
endif
ifeq ($(BUILD_CPU_MCU4_0),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=14 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu4_0.out.rprc
endif
ifeq ($(BUILD_CPU_MCU4_1),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=15 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu4_1.out.rprc
endif
ifeq ($(BUILD_CPU_C6x_1),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=16 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c6x_1.out.rprc
endif
ifeq ($(BUILD_CPU_C6x_2),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=17 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c6x_2.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=18 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_1.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=19 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_2.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_3),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=20 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_3.out.rprc
endif
ifeq ($(BUILD_CPU_C7x_4),yes)
    REMOTE_CORE_LIST_LATEAPP2_COMBINED+=21 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_4.out.rprc
endif


##############
### Main Target
##############
sbl: sbl_combined_bootimage
sbl_clean: sbl_scrub
sbl_scrub: sbl_combined_bootimage_clean

sbl_combined_bootimage: sbl_sd_hlos sbl_ospi_hlos sbl_appimage_hlos
	@echo "------------------------------------------------------"
	@echo "SBL Combined Boot Image generation completed."
	@echo "use 'make sbl_combined_bootimage_install_sd' to install the images to the SD card"
	@echo "------------------------------------------------------"

sbl_appimage_hlos: sbl_bootapp sbl_lateapps sbl_appimage

sbl_syscfg_gui:
	$(MAKE) -C $(PDK_PATH)/packages/ti/drv/pdm_utils/tools/dm_power_config_tool syscfg_gui BOARD=$(BOARD)
	$(MAKE) -C $(PDK_PATH)/packages/ti/drv/pdm_utils/tools/dm_power_config_tool syscfg BOARD=$(BOARD)

##############
### Install
##############
sbl_combined_bootimage_install_sd:
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "Installing QNX FS SBL to SD Card..."
	cp $(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage $(SBL_SD_FS_PATH)/atf_optee.appimage
	cp $(SBL_COMBINED_BOOTFILES_PATH)/ifs_qnx.appimage $(SBL_SD_FS_PATH)/ifs_qnx.appimage
endif
ifeq ($(BUILD_LINUX_MPU), yes)
	@echo "Installing LINUX FS SBL to SD Card..."
	cp $(ATF_BINARY_PATH) $(SBL_SD_FS_PATH)/atf_optee.appimage
	cp $(DTB_BINARY_PATH) $(SBL_SD_FS_PATH)/tidtb_linux.appimage
	cp $(KERNEL_BINARY_PATH) $(SBL_SD_FS_PATH)/tikernelimage_linux.appimage
endif
	cp ${LATEAPP1_BINARY_PATH} $(SBL_SD_FS_PATH)/lateapp1
	cp ${LATEAPP2_BINARY_PATH} $(SBL_SD_FS_PATH)/lateapp2
	cp $(SBL_COMBINED_BOOTFILES_PATH)/tiboot3.bin $(SBL_SD_FS_PATH)/tiboot3.bin
	cp $(SBL_COMBINED_BOOTFILES_PATH)/tifs.bin $(SBL_SD_FS_PATH)/tifs.bin
	cp $(SBL_COMBINED_BOOTFILES_PATH)/app $(SBL_SD_FS_PATH)/app
	sync
	@echo "Installation complete."

##############
### SBL -> generate tiboot3.bin and tifs.bin
##############
sbl_sd_hlos:
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)   
    ifeq ($(SOC_TYPE),hs)
		$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_mmcsd_img_hs TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
		cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)_hs/mmcsd/bin/sbl_mmcsd_img_$(SBL_CORE)_release.tiimage $(SBL_COMBINED_BOOTFILES_PATH)/tiboot3.bin
    else
		$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_mmcsd_img TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
		cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)/mmcsd/bin/sbl_mmcsd_img_$(SBL_CORE)_release.tiimage $(SBL_COMBINED_BOOTFILES_PATH)/tiboot3.bin
    endif
	cp $(TIFS_BINARY_PATH) $(SBL_COMBINED_BOOTFILES_PATH)/tifs.bin

sbl_ospi_hlos:
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)/ospi
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_ospi_img TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_cust_img TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
	cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)/ospi/bin/sbl_ospi_img_$(SBL_CORE)_release.tiimage $(SBL_COMBINED_BOOTFILES_PATH)/ospi/sbl_ospi_img_$(SBL_CORE)_release.tiimage
	cp $(PDK_PATH)/packages/ti/boot/sbl/binary/$(BOARD)/cust/bin/sbl_cust_img_$(SBL_CORE)_release.tiimage $(SBL_COMBINED_BOOTFILES_PATH)/ospi/sbl_cust_img_$(SBL_CORE)_release.tiimage
	cp $(PDK_PATH)/packages/ti/board/src/flash/nor/ospi/nor_spi_patterns.bin $(SBL_COMBINED_BOOTFILES_PATH)/ospi/nor_spi_patterns.bin
	cp $(TIFS_BINARY_PATH) $(SBL_COMBINED_BOOTFILES_PATH)/tifs.bin
	

#############
### Boot app -> generate app
#############

sbl_bootapp: sbl_bootapp_sd sbl_bootapp_ospi

sbl_bootapp_sd:
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)
    ifeq ($(BUILD_QNX_MPU), yes)
		$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_qnx ECU_BUILD=$(ECU_BUILD) TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
        ifeq ($(ECU_BUILD), no)
			cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/mmcsd/sbl_boot_app_mmcsd_qnx_$(BOARD)_$(SBL_CORE)_$(BUILD_OS_TYPE)_TestApp_release.appimage $(SBL_COMBINED_BOOTFILES_PATH)/app
        else
			cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/mmcsd/sbl_boot_app_mmcsd_$(ECU_BUILD)_qnx_$(BOARD)_$(SBL_CORE)_$(BUILD_OS_TYPE)_TestApp_release.appimage $(SBL_COMBINED_BOOTFILES_PATH)/app
        endif
    endif
    ifeq ($(BUILD_LINUX_MPU), yes)
        ifeq ($(SOC_TYPE),hs)
			$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_linux TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s BUILD_OS_TYPE=$(BUILD_OS_TYPE) BUILD_HS=yes HLOSBOOT=linux
        else
			$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_linux TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s BUILD_OS_TYPE=$(BUILD_OS_TYPE) HLOSBOOT=linux
        endif
		cp $(BOOTAPP_IMAGE_PATH) $(SBL_COMBINED_BOOTFILES_PATH)/app
    endif

sbl_bootapp_ospi:
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)/ospi
    ifeq ($(BUILD_QNX_MPU), yes)
		$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_ospi_qnx ECU_BUILD=$(ECU_BUILD) TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
        ifeq ($(ECU_BUILD), no)
			cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/ospi/sbl_boot_app_ospi_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage $(SBL_COMBINED_BOOTFILES_PATH)/ospi/sbl_boot_app_ospi_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage
        else
			cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/ospi/sbl_boot_app_ospi_$(ECU_BUILD)_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage $(SBL_COMBINED_BOOTFILES_PATH)/ospi/sbl_boot_app_ospi_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage
        endif
    endif
    ifeq ($(BUILD_LINUX_MPU), yes)
		$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_ospi_linux TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) DISABLE_RECURSE_DEPS=no BOARD=$(BOARD) CORE=$(SBL_CORE) -s
		cp $(PDK_PATH)/packages/ti/boot/sbl/example/boot_app/binary/$(BOARD)/ospi/sbl_boot_app_ospi_linux_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage $(SBL_COMBINED_BOOTFILES_PATH)/ospi/sbl_boot_app_ospi_linux_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage
    endif

#############
### Lateapps -> generate lateapp1 and lateapp2
#############

sbl_vision_apps_rprc:
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)/rprcs

    # Generate RPRC files for each core's output
    ifeq ($(BUILD_CPU_MCU2_0),yes)
		$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu2_0.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu2_0.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu2_0.out.rprc
    endif
    ifeq ($(BUILD_CPU_MCU2_1),yes)
		$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu2_1.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu2_1.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu2_1.out.rprc
    endif
    ifeq ($(BUILD_CPU_MCU3_0),yes)
		$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu3_0.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu3_0.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu3_0.out.rprc
    endif
    ifeq ($(BUILD_CPU_MCU3_1),yes)
		$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu3_1.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu3_1.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu3_1.out.rprc
    endif
    ifeq ($(BUILD_CPU_MCU4_0),yes)
		$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu4_0.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu4_0.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu4_0.out.rprc
    endif
    ifeq ($(BUILD_CPU_MCU4_1),yes)
		$(TIARMCGT_LLVM_ROOT)/bin/tiarmstrip -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu4_1.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_mcu4_1.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_mcu4_1.out.rprc
    endif
    ifeq ($(BUILD_CPU_C6x_1),yes)
		$(CGT6X_ROOT)/bin/strip6x -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c6x_1.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c6x_1.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c6x_1.out.rprc
    endif
    ifeq ($(BUILD_CPU_C6x_2),yes)
		$(CGT6X_ROOT)/bin/strip6x -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c6x_2.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C66/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c6x_2.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c6x_2.out.rprc
    endif
    ifeq ($(BUILD_CPU_C7x_1),yes)
		$(CGT7X_ROOT)/bin/strip7x -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_1.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_1.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_1.out.rprc
    endif
    ifeq ($(BUILD_CPU_C7x_2),yes)
		$(CGT7X_ROOT)/bin/strip7x -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_2.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_2.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_2.out.rprc
    endif
    ifeq ($(BUILD_CPU_C7x_3),yes)
		$(CGT7X_ROOT)/bin/strip7x -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_3.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_3.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_3.out.rprc
    endif
    ifeq ($(BUILD_CPU_C7x_4),yes)
		$(CGT7X_ROOT)/bin/strip7x -p $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_4.out
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(C7X_TARGET)/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS)_c7x_4.out $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/vx_app_rtos_$(HLOS)_c7x_4.out.rprc
    endif

sbl_lateapps: sbl_vision_apps_rprc
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/lateapp1 $(REMOTE_CORE_LIST_LATEAPP1_COMBINED)
	$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/lateapp2 $(REMOTE_CORE_LIST_LATEAPP2_COMBINED)
    ifeq ($(SOC_TYPE),hs)
		${CERT_SCRIPT} -b $(SBL_COMBINED_BOOTFILES_PATH)/lateapp1 -o $(SBL_COMBINED_BOOTFILES_PATH)/lateapp1.signed -c R5 -l 0x41C00100 -k ${PDK_PATH}/packages/ti/build/makerules/k3_dev_mpk.pem
		${CERT_SCRIPT} -b $(SBL_COMBINED_BOOTFILES_PATH)/lateapp2 -o $(SBL_COMBINED_BOOTFILES_PATH)/lateapp2.signed -c R5 -l 0x41C00100 -k ${PDK_PATH}/packages/ti/build/makerules/k3_dev_mpk.pem
    else ifeq ($(SOC_TYPE),hs_fs)
		${CERT_SCRIPT} -b $(SBL_COMBINED_BOOTFILES_PATH)/lateapp1 -o $(SBL_COMBINED_BOOTFILES_PATH)/lateapp1.hs_fs -c R5 -l 0x41C00100 -k ${PDK_PATH}/packages/ti/build/makerules/rom_degenerateKey.pem
		${CERT_SCRIPT} -b $(SBL_COMBINED_BOOTFILES_PATH)/lateapp2 -o $(SBL_COMBINED_BOOTFILES_PATH)/lateapp2.hs_fs -c R5 -l 0x41C00100 -k ${PDK_PATH}/packages/ti/build/makerules/rom_degenerateKey.pem
    endif

##############
### QNX appimage -> Generate ifs_qnx.appimage and atf_optee.appimage
##############
### LINUX appimage -> Generate atf_optee.appimage, tidtb_linux.appimage and tikernelimage_linux.appimage
##############
sbl_appimage: sbl_combined_atf_optee
    ifeq ($(BUILD_QNX_MPU), yes)
		mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)/rprcs
		curr_dir=$(PWD)
    ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
		@echo "Building QNX AppImage with ATF + OPTEE"
		cd $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir && \
		$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/atf_optee.lds -o $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_optee.elf && \
		cd $(QNX_BOOT_PATH) && \
		$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/ifs_qnx.lds -o $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.elf && \
		cd $(curr_dir)
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_optee.elf $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_optee.rprc
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.elf $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
		$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage 0 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_optee.rprc
		$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/ifs_qnx.appimage 0 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
    else
		@echo "Building QNX AppImage with ATF only"
		cd $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir && \
		$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/atf_only.lds -o $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_only.elf && \
		cd $(QNX_BOOT_PATH) && \
		$(QNX_BASE)/host/linux/x86_64/usr/bin/$(QNX_CROSS_COMPILER_TOOL)ld -T $(LDS_PATH)/ifs_qnx.lds -o $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.elf && \
		cd $(curr_dir)
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_only.elf $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_only.rprc
		mono $(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.elf $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
		$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage 0 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/atf_only.rprc
		$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/ifs_qnx.appimage 0 $(SBL_COMBINED_BOOTFILES_PATH)/rprcs/ifs_qnx.rprc
    endif
    endif
    ifeq ($(BUILD_LINUX_MPU), yes)
		@echo "Generating Linux appimages..."
		curr_dir=$(PWD)

		@echo "Decompiling k3-${SOC}-evm.dtb to temporary k3-${SOC}-evm-temp.dts..."
		dtc -I dtb -O dts -o $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm-temp.dts $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm.dtb

		@echo "Patching k3-${SOC}-evm-temp.dts..."
		@dtb_source=$(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm-temp.dts; \
		if [ "$(SOC)" = "j784s4" ] || [ "$(SOC)" = "j742s2" ] || [ "$(SOC)" = "j721s2" ]; then \
			sed -i '/chosen {/a\ \ \ \ \ \ \ \ bootargs = "console=ttyS2,115200n8 earlycon=ns16550a,mmio32,0x2880000 root=/dev/mmcblk1p2 rw rootfstype=ext4 rootwait";' "$$dtb_source"; \
		elif [ "$(SOC)" = "j721e" ]; then \
			sed -i '/chosen {/a\ \ \ \ \ \ \ \ bootargs = "console=ttyS2,115200n8 earlycon=ns16550a,mmio32,0x02800000 root=/dev/mmcblk1p2 rw rootfstype=ext4 rootwait";' "$$dtb_source"; \
		fi

		@echo "Recompiling k3-${SOC}-evm-temp.dts to temporary k3-${SOC}-evm-temp.dtb..."
		dtc -I dts -O dtb -o $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm-temp.dtb $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm-temp.dts

		cd $(PSDK_PATH)/targetfs/boot/dtb/ti && \
		if [ "$(SOC)" = "j784s4" ] || [ "$(SOC)" = "j721e" ]; then \
			fdtoverlay -i k3-${SOC}-evm-temp.dtb -o base-board.dtb k3-${SOC}-evm-ethfw.dtbo k3-${SOC}-vision-apps.dtbo; \
		elif [ "$(SOC)" = "j721s2" ] || [ "$(SOC)" = "j742s2" ]; then \
			fdtoverlay -i k3-${SOC}-evm-temp.dtb -o base-board.dtb k3-${SOC}-vision-apps.dtbo; \
		fi; \
		\
		if [ "$(SOC)" = "j721e" ]; then \
			echo "Disabling SerDes WIZ and CPSW nodes for J721E SBL+ETHFW..."; \
			fdtput -t s base-board.dtb /bus@100000/wiz@5000000 status "disabled" || true; \
			fdtput -t s base-board.dtb /bus@100000/wiz@5010000 status "disabled" || true; \
			fdtput -t s base-board.dtb /bus@100000/wiz@5020000 status "disabled" || true; \
			fdtput -t s base-board.dtb /bus@100000/wiz@5030000 status "disabled" || true; \
			fdtput -t s base-board.dtb /bus@100000/ethernet@c000000 status "disabled" || true; \
		fi; \

		@echo "Cleaning up temporary files..."
		rm -f $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm-temp.dts
		rm -f $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-${SOC}-evm-temp.dtb

		cd $(curr_dir)
		cp $(PSDK_PATH)/targetfs/boot/dtb/ti/base-board.dtb $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir/
		cp $(PSDK_PATH)/targetfs/boot/Image $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir/

		$(eval GENFILES := atf_optee tidtb_linux tikernelimage_linux)
		mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)
		cd $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir && \
		for i in $(GENFILES); do \
			echo "Generating $$i image"; \
			${PSDK_TOOLS_PATH}/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-ld -T $(LDS_PATH)/$$i.lds -o $(SBL_COMBINED_BOOTFILES_PATH)/$$i.elf; \
			$(SBL_OUT2RPRC_GEN_TOOL_PATH)/out2rprc.exe $(SBL_COMBINED_BOOTFILES_PATH)/$$i.elf $(SBL_COMBINED_BOOTFILES_PATH)/$$i.rprc; \
			$(MULTICORE_APPIMAGE_GEN_TOOL_PATH)/MulticoreImageGen LE $(DEV_ID) $(SBL_COMBINED_BOOTFILES_PATH)/$$i.appimage 0 $(SBL_COMBINED_BOOTFILES_PATH)/$$i.rprc; \
			rm -rf $(SBL_COMBINED_BOOTFILES_PATH)/$$i.elf $(SBL_COMBINED_BOOTFILES_PATH)/$$i.rprc; \
		done; \
		cd $(curr_dir)
        ifeq ($(SOC_TYPE),hs)
			for i in $(GENFILES); do \
				$(CERT_SCRIPT) -b $(SBL_COMBINED_BOOTFILES_PATH)/$$i.appimage -o $(SBL_COMBINED_BOOTFILES_PATH)/$$i.appimage.signed -c R5 -l 0x41C00100 -k $(PDK_PATH)/packages/ti/build/makerules/k3_dev_mpk.pem; \
			done
        else ifeq ($(SOC_TYPE),hs_fs)
			for i in $(GENFILES); do \
				$(CERT_SCRIPT) -b $(SBL_COMBINED_BOOTFILES_PATH)/$$i.appimage -o $(SBL_COMBINED_BOOTFILES_PATH)/$$i.appimage.hs_fs -c R5 -l 0x41C00100 -k $(PDK_PATH)/packages/ti/build/makerules/rom_degenerateKey.pem; \
			done
        endif
    endif

sbl_combined_atf_optee:
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir
    ifeq ($(BUILD_QNX_MPU), yes)
    ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
		# For ATF, setting HANDLE_EA_EL3_FIRST_NS=0 for QNX so that the all runtime exception to be routed to current exception level (or in EL1 if the current exception level is EL0)
		$(MAKE) -C $(VISION_APPS_PATH)/../trusted-firmware-a -s -j32 CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" PLAT=k3 TARGET_BOARD=$(ATF_TARGET_BOARD) SPD=opteed HANDLE_EA_EL3_FIRST_NS=0 K3_USART=$(K3_USART)
    else
		# For ATF, setting HANDLE_EA_EL3_FIRST_NS=0 for QNX so that the all runtime exception to be routed to current exception level (or in EL1 if the current exception level is EL0)
		$(MAKE) -C $(VISION_APPS_PATH)/../trusted-firmware-a -s -j32 CROSS_COMPILE=$(CROSS_COMPILE) CC="$(CROSS_COMPILE)gcc --sysroot=$(LINUX_SYSROOT_ARM)" PLAT=k3 TARGET_BOARD=$(ATF_TARGET_BOARD) HANDLE_EA_EL3_FIRST_NS=0 K3_USART=$(K3_USART)
    endif
		cp $(VISION_APPS_PATH)/../trusted-firmware-a/build/k3/$(ATF_TARGET_BOARD)/release/bl31.bin $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir/bl31.bin
    endif

    ifeq ($(BUILD_LINUX_MPU), yes)
		cp ${LINUX_BUILD_DIR_PATH}/bl31.bin $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir/bl31.bin
    endif

    ifeq ($(USE_OPTEE),$(filter $(USE_OPTEE), 1))
		cp ${LINUX_BUILD_DIR_PATH}/bl32.bin $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir/bl32.bin
    endif


##############
### clean up
##############
sbl_combined_bootimage_clean: sbl_combined_bootimage_scrub
sbl_combined_bootimage_scrub: sbl_sd_hlos_clean sbl_ospi_hlos_clean sbl_bootapp_clean sbl_lateapps_clean sbl_appimage_clean
	rm -rf $(VISION_APPS_PATH)/out/sbl_combined_bootfiles/

sbl_sd_hlos_clean:
    ifeq ($(SOC_TYPE),hs)
		$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_mmcsd_img_hs_clean TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) BOARD=$(BOARD) CORE=$(SBL_CORE)
    else
		$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_mmcsd_img_clean TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) BOARD=$(BOARD) CORE=$(SBL_CORE)
    endif
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/tiboot3.bin
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/tifs.bin

sbl_ospi_hlos_clean:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build sbl_ospi_img_clean TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) BOARD=$(BOARD) CORE=$(SBL_CORE)
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/sbl_boot_app_ospi_qnx_$(BOARD)_$(SBL_CORE)_freertos_TestApp_release.appimage
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/tifs.bin

sbl_bootapp_clean:
    ifeq ($(BUILD_QNX_MPU), yes)
		$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_qnx_clean TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) BOARD=$(BOARD) CORE=$(SBL_CORE) -s
    endif
    ifeq ($(BUILD_LINUX_MPU), yes)
        ifeq ($(SOC_TYPE),hs)
			$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_hs_clean TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) BOARD=$(BOARD) CORE=$(SBL_CORE) -s
        else
			$(MAKE) -C $(PDK_PATH)/packages/ti/build boot_app_mmcsd_linux_clean TOOLS_INSTALL_PATH=$(PSDK_TOOLS_PATH) BOARD=$(BOARD) CORE=$(SBL_CORE) -s
        endif
    endif
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/app

sbl_lateapps_clean:
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/lateapp*
	rm -rf $(SBL_COMBINED_BOOTFILES_PATH)/rprcs

sbl_appimage_clean:
	rm -rf $(SBL_COMBINED_BOOTFILES_PATH)/sbl_appimage_stage_dir
    ifeq ($(BUILD_QNX_MPU), yes)
		$(MAKE) -C $(VISION_APPS_PATH)/../trusted-firmware-a clean
		rm -f $(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage
		rm -f $(SBL_COMBINED_BOOTFILES_PATH)/ifs_qnx.appimage
		rm -rf $(VISION_APPS_PATH)/../trusted-firmware-a/build/k3/$(ATF_TARGET_BOARD)
    endif
    ifeq ($(BUILD_LINUX_MPU), yes)
		rm -f $(SBL_COMBINED_BOOTFILES_PATH)/atf_optee.appimage*
		rm -f $(SBL_COMBINED_BOOTFILES_PATH)/tidtb_linux.appimage*
		rm -f $(SBL_COMBINED_BOOTFILES_PATH)/tikernelimage_linux.appimage*
    endif

endif # ifeq ($(RTOS_SDK),pdk)

#######################################################################################
#################################[ MCU PLUS SDK SBL ]##################################

# Check if the RTOS SDK is set to mcu_plus_sdk and BUILD_QNX_MPU or BUILD_LINUX_MPU is enabled
ifeq ($(RTOS_SDK),mcu_plus_sdk)

# Include the configuration file for the specified SOC based on HLOS type
# Use soft includes (-include) to avoid errors when MCU_PLUS_SDK_PATH is not available (e.g., Yocto builds)
ifeq ($(BUILD_QNX_MPU), yes)
    # Include QNX configuration
    ifeq ($(SOC), am62a)
	include $(MCU_PLUS_SDK_PATH)/tools/boot/qnxAppimageGen/board/am62ax-sk/config.mak
    else
	include $(MCU_PLUS_SDK_PATH)/tools/boot/qnxAppimageGen/board/$(SOC)-evm/config.mak
    endif
else ifeq ($(BUILD_LINUX_MPU), yes)
    # Include Linux configuration
    ifeq ($(SOC), j722s)
    # FASTBOOT_LINUX boot mode selection (set BEFORE including config.mak):
    #   0: SBL → ATF+OPTEE+U-Boot SPL → U-Boot → Linux (requires DTB on SD card)
    #   1: SBL → ATF+OPTEE+Kernel+DTB directly (no U-Boot, DTB in appimage)
    # Default: 0 (U-Boot flow) - change to 1 for direct Linux boot
	-include $(MCU_PLUS_SDK_PATH)/tools/boot/linuxAppimageGen/board/$(SOC)-evm/config.mak
    endif
endif

# Setting OS type (like freertos, safertos) here in lowercase letter
BUILD_OS_TYPE?=$(shell echo $(RTOS) | tr '[:upper:]' '[:lower:]')

# Setting HLOS and APP_PROFILE variable based on build type
ifeq ($(BUILD_QNX_MPU), yes)
HLOS := qnx
else ifeq ($(BUILD_LINUX_MPU), yes)
HLOS := linux
endif

ifeq ($(HLOS),qnx)
APP_PROFILE := $(QNX_APP_PROFILE)
else ifeq ($(HLOS),linux)
APP_PROFILE := $(LINUX_APP_PROFILE)
endif

# Define paths and commands for generating RPRC images and SBL binaries
SBL_COMBINED_BOOTFILES_PATH = $(VISION_APPS_PATH)/out/sbl_combined_bootfiles
OUTRPRC_CMD = $(SYSCFG_NODE) $(MCU_PLUS_SDK_PATH)/tools/boot/out2rprc/elf2rprc.js
INPUT_IMG_PATH = $(VISION_APPS_PATH)/out/sbl_combined_bootfiles/vision_apps

ifeq ($(ECU_BUILD), no)
SBL_SD_HLOS_PATH = $(MCU_PLUS_SDK_PATH)/examples/drivers/boot/sbl_sd_hlos/$(SOC)-evm/wkup-r5fss0-0_nortos/ti-arm-clang
SBL_SD_HLOS_OUT_IMG = $(SBL_SD_HLOS_PATH)/sbl_sd_hlos.$(APP_PROFILE).hs_fs.tiimage
SBL_OSPI_HLOS_PATH = $(MCU_PLUS_SDK_PATH)/examples/drivers/boot/sbl_ospi_hlos/$(SOC)-evm/wkup-r5fss0-0_nortos/ti-arm-clang
SBL_OSPI_HLOS_OUT_IMG = $(SBL_OSPI_HLOS_PATH)/sbl_ospi_hlos.$(APP_PROFILE).hs_fs.tiimage
else
SBL_SD_HLOS_PATH = $(MCU_PLUS_SDK_PATH)/examples/drivers/boot/sbl_sd_hlos_${ECU_BUILD}/$(SOC)-evm/wkup-r5fss0-0_nortos/ti-arm-clang
SBL_SD_HLOS_OUT_IMG = $(SBL_SD_HLOS_PATH)/sbl_sd_hlos_${ECU_BUILD}.$(APP_PROFILE).hs_fs.tiimage
SBL_OSPI_HLOS_PATH = $(MCU_PLUS_SDK_PATH)/examples/drivers/boot/sbl_ospi_hlos_${ECU_BUILD}/$(SOC)-evm/wkup-r5fss0-0_nortos/ti-arm-clang
SBL_OSPI_HLOS_OUT_IMG = $(SBL_OSPI_HLOS_PATH)/sbl_ospi_hlos_${ECU_BUILD}.$(APP_PROFILE).hs_fs.tiimage
endif

SBL_UART_UNIFLASH_PATH = $(MCU_PLUS_SDK_PATH)/examples/drivers/boot/sbl_uart_uniflash/$(SOC)-evm/wkup-r5fss0-0_nortos/ti-arm-clang/
SBL_UART_UNIFLASH_IMG = $(SBL_UART_UNIFLASH_PATH)/sbl_uart_uniflash.$(APP_PROFILE).hs_fs.tiimage

# HLOS specific appimage paths
ifeq ($(BUILD_QNX_MPU), yes)
HLOS_TYPE = qnx
QNX_COMBINED_APP_IMG = $(MCU_PLUS_SDK_PATH)/tools/boot/qnxAppimageGen/board/$(SOC)-evm/qnx.appimage.hs_fs
HLOS_COMBINED_APP_IMG = $(QNX_COMBINED_APP_IMG)
HLOS_APPIMAGE_NAME = qnx.appimage.hs_fs
else ifeq ($(BUILD_LINUX_MPU), yes)
HLOS_TYPE = linux
SBL_SD_FS_PATH = $(LINUX_SD_FS_BOOT_PATH)
LINUX_COMBINED_APP_IMG = $(MCU_PLUS_SDK_PATH)/tools/boot/linuxAppimageGen/board/$(SOC)-evm/linux.appimage.hs_fs
HLOS_COMBINED_APP_IMG = $(LINUX_COMBINED_APP_IMG)
HLOS_APPIMAGE_NAME = linux.appimage.hs_fs
endif

# Ensure required variables are defined, else throw an error
ifndef MCU_PLUS_SDK_PATH
    $(error MCU_PLUS_SDK_PATH is not defined)
endif
ifndef VISION_APPS_PATH
    $(error VISION_APPS_PATH is not defined)
endif
ifndef SOC
    $(error SOC is not defined)
endif
ifeq ($(BUILD_QNX_MPU), yes)
    ifndef QNX_BOOT_PATH
        $(error QNX_BOOT_PATH is not defined)
    endif
    ifndef QNX_SD_FS_BOOT_PATH
        $(error QNX_SD_FS_BOOT_PATH is not defined)
    endif
else ifeq ($(BUILD_LINUX_MPU), yes)
    ifndef PSDK_PATH
        $(warning PSDK_PATH is not defined, Linux binaries may not be found)
    endif
    ifndef SBL_SD_FS_PATH
        $(warning SBL_SD_FS_PATH is not defined, SD card installation will not work)
    endif
endif

# Define input images for R5F and C7x cores
IMAGE_MCU2_0 = $(VISION_APPS_PATH)/out/$(TARGET_SOC)/R5F/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS_TYPE)_mcu2_0.out
IMAGE_C7X_1 = $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7524/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS_TYPE)_c7x_1.out
IMAGE_C7X_2 = $(VISION_APPS_PATH)/out/$(TARGET_SOC)/C7524/$(RTOS)/$(APP_PROFILE)/vx_app_rtos_$(HLOS_TYPE)_c7x_2.out

# Define output RPRC files for R5F and C7x cores
RPRC_MCU2_0 = $(INPUT_IMG_PATH)/vx_app_rtos_$(HLOS_TYPE)_mcu2_0.rprc
RPRC_C7X_1 = $(INPUT_IMG_PATH)/vx_app_rtos_$(HLOS_TYPE)_c7x_1.rprc
RPRC_C7X_2 = $(INPUT_IMG_PATH)/vx_app_rtos_$(HLOS_TYPE)_c7x_2.rprc

# Define the list of images to be included in the RTOS image list
IMG1 = $(BOOTIMAGE_CORE_ID_wkup-r5fss0-0) $(MCU_PLUS_SDK_PATH)/tools/sysfw/sciserver_binary/$(SOC)/sciclient_get_version.$(APP_PROFILE).rprc
ifeq ($(BUILD_CPU_MCU2_0),yes)
    IMG2=$(BOOTIMAGE_CORE_ID_main-r5fss0-0) $(RPRC_MCU2_0)
else
    IMG2=
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
    IMG3 = $(BOOTIMAGE_CORE_ID_c75ss0-0) $(RPRC_C7X_1)
else
    IMG3 =
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
    IMG4 = $(BOOTIMAGE_CORE_ID_c75ss1-0) $(RPRC_C7X_2)
else
    IMG4 =
endif

RTOS_IMG_LIST = $(IMG1) $(IMG2) $(IMG3) $(IMG4)

# Build the SBL and app image
sbl: sbl_appimage_hlos sbl_sd_hlos sbl_ospi_hlos

# Clean up generated files
sbl_clean: sbl_scrub
sbl_scrub: sbl_appimage_hlos_clean sbl_sd_hlos_clean sbl_ospi_hlos_clean
	rm -rf $(SBL_COMBINED_BOOTFILES_PATH)

# Generate the SBL MMC SD binary
sbl_sd_hlos:
	@echo "Generating SBL MMC SD Binary..."
	make -C $(SBL_SD_HLOS_PATH) PROFILE=$(APP_PROFILE) clean
	make -C $(SBL_SD_HLOS_PATH) PROFILE=$(APP_PROFILE)

	@echo "Copying SBL SD HLOS binary to combined bootfiles path..."
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)
	cp $(SBL_SD_HLOS_OUT_IMG) $(SBL_COMBINED_BOOTFILES_PATH)/sbl_sd_hlos.$(APP_PROFILE).hs_fs.tiimage

# Generate the OSPI binary
sbl_ospi_hlos:
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "Generating SBL OSPI Binary..."
	make -C $(SBL_OSPI_HLOS_PATH) PROFILE=$(APP_PROFILE) clean
	make -C $(SBL_OSPI_HLOS_PATH) PROFILE=$(APP_PROFILE)
	make -C ${SBL_UART_UNIFLASH_PATH} PROFILE=$(APP_PROFILE) clean
	make -C ${SBL_UART_UNIFLASH_PATH} PROFILE=$(APP_PROFILE)

	@echo "Copying SBL OSPI HLOS binary to combined bootfiles path..."
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)
	cp $(SBL_OSPI_HLOS_OUT_IMG) $(SBL_COMBINED_BOOTFILES_PATH)/sbl_ospi_hlos.$(APP_PROFILE).hs_fs.tiimage
	cp $(SBL_UART_UNIFLASH_IMG) $(SBL_COMBINED_BOOTFILES_PATH)/sbl_uart_uniflash.$(APP_PROFILE).hs_fs.tiimage
endif

# Generate RPRC images for vision apps
sbl_vision_apps_rprc:
	mkdir -p $(INPUT_IMG_PATH)
	@echo "SOC=$(SOC)"
	@echo "SW_VERSION=$(SW_VERSION)"
	$(call GENERATE_OUT_TO_RPRC, MCU2_0, $(IMAGE_MCU2_0), $(RPRC_MCU2_0))
	$(call GENERATE_OUT_TO_RPRC, C7X_1, $(IMAGE_C7X_1), $(RPRC_C7X_1))
	$(call GENERATE_OUT_TO_RPRC, C7X_2, $(IMAGE_C7X_2), $(RPRC_C7X_2))
	@echo "RPRC generation completed."

# Generate the combined app image for QNX or Linux
sbl_appimage_hlos: sbl_vision_apps_rprc
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "------------------------------------------------------"
	@echo "Generating combined app image for QNX..."
	@echo "------------------------------------------------------"

	@echo "Copying QNX IFS image to prebuilt bin path..."
	mkdir -p ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/qnx/
	cp ${QNX_BOOT_PATH}/qnx-ifs ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/qnx/;

	@echo "Generating multicore hlos qnx app image..."
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/tools/boot/qnxAppimageGen BOARD=${SOC}-evm clean
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/tools/boot/qnxAppimageGen BOARD=${SOC}-evm RTOS_IMG_LIST="$(RTOS_IMG_LIST)"

	@echo "------------------------------------------------------"
	@echo "multicore hlos qnx app image generated successfully."
	@echo "------------------------------------------------------"

	@echo "Copying generated qnx app image to combined bootfiles path..."
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)
	cp ${QNX_COMBINED_APP_IMG} $(SBL_COMBINED_BOOTFILES_PATH)/qnx.appimage.hs_fs
else ifeq ($(BUILD_LINUX_MPU), yes) 
ifeq ($(SOC), j722s)
	@echo "------------------------------------------------------"
	@echo "Generating combined app image for Linux..."
	@echo "------------------------------------------------------"
	@echo "Copying Linux binaries to prebuilt bin path..."
	mkdir -p ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/
	@if [ -n "$(PSDK_PATH)" ] && [ -d "$(PSDK_PATH)/targetfs/boot" ]; then \
		echo "Copying Linux kernel from PSDK..."; \
		cp $(PSDK_PATH)/targetfs/boot/Image ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/ 2>/dev/null || echo "Warning: Image not found"; \
		echo "Merging base DTB with vision apps overlay..."; \
		if [ -f "$(PSDK_PATH)/targetfs/boot/dtb/ti/k3-$(SOC)-vision-apps.dtbo" ]; then \
			fdtoverlay -i $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-$(SOC)-evm.dtb \
				-o ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/k3-$(SOC)-evm.dtb \
				$(PSDK_PATH)/targetfs/boot/dtb/ti/k3-$(SOC)-vision-apps.dtbo && \
			echo "DTB merged with vision apps overlay"; \
		else \
			echo "Warning: Vision apps overlay not found, copying base DTB"; \
			cp $(PSDK_PATH)/targetfs/boot/dtb/ti/k3-$(SOC)-evm.dtb ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/ 2>/dev/null || echo "Warning: DTB not found"; \
		fi; \
	else \
		echo "Warning: PSDK_PATH not set or targetfs/boot not found. Assuming Linux binaries are already in place."; \
	fi

	@echo "Generating multicore hlos linux app image..."
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/tools/boot/linuxAppimageGen BOARD=${SOC}-evm clean
	$(MAKE) -C $(MCU_PLUS_SDK_PATH)/tools/boot/linuxAppimageGen BOARD=${SOC}-evm RTOS_IMG_LIST="$(RTOS_IMG_LIST)"

	@echo "------------------------------------------------------"
	@echo "multicore hlos linux app image generated successfully."
	@echo "------------------------------------------------------"

	@echo "Copying generated linux app image to combined bootfiles path..."
	mkdir -p $(SBL_COMBINED_BOOTFILES_PATH)
	cp ${LINUX_COMBINED_APP_IMG} $(SBL_COMBINED_BOOTFILES_PATH)/linux.appimage.hs_fs

	@echo "Copying u-boot.img to combined bootfiles path..."
	@if [ -f "${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/u-boot.img" ]; then \
		cp ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/u-boot.img $(SBL_COMBINED_BOOTFILES_PATH)/; \
		echo "u-boot.img copied successfully"; \
	else \
		echo "Warning: u-boot.img not found at ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/u-boot.img"; \
	fi
endif 
endif

# Clean up app image files
sbl_appimage_hlos_clean:
ifeq ($(BUILD_QNX_MPU), yes)
	rm -f ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/qnx/qnx-ifs
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/qnx.appimage.hs_fs
else ifeq ($(BUILD_LINUX_MPU), yes)
ifeq ($(SOC), j722s)
	rm -f ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/Image
	rm -f ${MCU_PLUS_SDK_PATH}/tools/boot/hlos_prebuilt/${SOC}-evm/linux/k3-$(SOC)-evm.dtb
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/linux.appimage.hs_fs
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/u-boot.img
endif
endif
	rm -rf $(INPUT_IMG_PATH)

# Clean up SBL SD HLOS files
sbl_sd_hlos_clean:
	make -C $(SBL_SD_HLOS_PATH) clean
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/sbl_sd_hlos.$(APP_PROFILE).hs_fs.tiimage

# Clean up SBL OSPI HLOS files
sbl_ospi_hlos_clean:
	make -C $(SBL_OSPI_HLOS_PATH) clean
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/sbl_ospi_hlos.$(APP_PROFILE).hs_fs.tiimage
	rm -f $(SBL_COMBINED_BOOTFILES_PATH)/sbl_uart_uniflash.$(APP_PROFILE).hs_fs.tiimage

# Copy SBL and HLOS app image to SD card
sbl_bootimage_install_sd:
ifeq ($(BUILD_QNX_MPU), yes)
	@echo "Copying SBL SD HLOS for QNX to SD card..."
	cp $(SBL_COMBINED_BOOTFILES_PATH)/sbl_sd_hlos.$(APP_PROFILE).hs_fs.tiimage $(QNX_SD_FS_BOOT_PATH)/tiboot3.bin

	@echo "Copying QNX app image to SD card..."
	cp $(SBL_COMBINED_BOOTFILES_PATH)/qnx.appimage.hs_fs $(QNX_SD_FS_BOOT_PATH)/app
	sync

	@echo "SBL tiboot3.bin and QNX app image copied to SD card successfully."
else ifeq ($(BUILD_LINUX_MPU), yes)
ifeq ($(SOC), j722s)
	@echo "=========================================="
	@echo "Installing SBL Linux Boot to SD Card"
	@echo "=========================================="
	@echo ""
	@if [ -z "$(SBL_SD_FS_PATH)" ]; then \
		echo "Error: SBL_SD_FS_PATH is not defined. Cannot install to SD card."; \
		echo "Please set: export SBL_SD_FS_PATH=/media/<user>/boot"; \
		exit 1; \
	fi
	cp $(SBL_COMBINED_BOOTFILES_PATH)/sbl_sd_hlos.$(APP_PROFILE).hs_fs.tiimage $(SBL_SD_FS_PATH)/tiboot3.bin

	@echo "Copying Linux app image to SD card..."
	cp $(SBL_COMBINED_BOOTFILES_PATH)/linux.appimage.hs_fs $(SBL_SD_FS_PATH)/app

	@echo "Copying u-boot.img to SD card (U-Boot flow)..."
	@if [ -f "$(SBL_COMBINED_BOOTFILES_PATH)/u-boot.img" ]; then \
		cp $(SBL_COMBINED_BOOTFILES_PATH)/u-boot.img $(SBL_SD_FS_PATH)/; \
		echo "u-boot.img copied"; \
	else \
		echo "Warning: u-boot.img not found"; \
	fi

	@echo "Checking uEnv.txt for SBL boot compatibility..."
	@if [ -f "$(SBL_SD_FS_PATH)/uEnv.txt" ]; then \
		if grep -q "^dorprocboot=1" $(SBL_SD_FS_PATH)/uEnv.txt; then \
			echo "Fixing dorprocboot: 1 to 0 (remote cores loaded by SBL)"; \
			sed -i 's/^dorprocboot=1/dorprocboot=0/' $(SBL_SD_FS_PATH)/uEnv.txt; \
			echo "uEnv.txt updated: dorprocboot=0"; \
		elif grep -q "^dorprocboot=0" $(SBL_SD_FS_PATH)/uEnv.txt; then \
			echo "uEnv.txt already has dorprocboot=0"; \
		else \
			echo "Warning: uEnv.txt exists but no dorprocboot setting found"; \
		fi; \
	else \
		echo "Warning: uEnv.txt not found on SD card"; \
		echo "Please ensure uEnv.txt exists with dorprocboot=0 and name_overlays=ti/k3-$(SOC)-vision-apps.dtbo"; \
	fi
	sync
endif
endif

# Flash SBL OSPI HLOS to the device
sbl_bootimage_install_ospi:
	@echo "Flashing SBL OSPI HLOS for ECU build to device..."
	@read -p "Enter UART port (default: /dev/ttyUSB2): " input; \
	UART_FLASH_PORT=$${input:-/dev/ttyUSB2}; \
	echo "Using UART port: $$UART_FLASH_PORT"; \
	cd $(MCU_PLUS_SDK_PATH)/tools/boot && \
	python uart_uniflash.py -p $$UART_FLASH_PORT --cfg=sbl_prebuilt/j722s-evm/default_sbl_ospi_qnx_hs_fs_fc.cfg

# Macro to generate RPRC images
define GENERATE_OUT_TO_RPRC =
	@echo "Creating $(1) RPRC image"
	@if [ -f $(2) ]; then \
		$(OUTRPRC_CMD) $(2) $(SW_VERSION) $(3); \
		echo "$(1) RPRC image created: $(3)"; \
	else \
		echo "Warning: $(1) image $(2) does not exist. Skipping RPRC creation."; \
	fi
endef

endif  # ifeq ($(RTOS_SDK),mcu_plus_sdk)
