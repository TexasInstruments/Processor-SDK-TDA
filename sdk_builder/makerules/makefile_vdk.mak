#
# Utility makefile to generate vdk output
#

ifeq ($(BUILD_LINUX_MPU),yes)
BUILT_IMAGES?=$(PSDK_PATH)/out
VISION_APPS_TARGET_PATH?=$(VISION_APPS_PATH)/out/$(TARGET_SOC)
VISION_APPS_PC_PATH?=$(VISION_APPS_PATH)/out/PC/x86_64/LINUX
ROOTFS?=$(PSDK_PATH)/tisdk-adas-image-tda54-vdk.tar.xz
MOUNT_POINT?=${BUILT_IMAGES}/mnt
LOOP_DEV=$(shell losetup -f)
BUILD_PROFILE?=release
# end of ifeq ($(BUILD_LINUX_MPU),yes)
else ifeq ($(BUILD_QNX_MPU),yes)
BUILT_IMAGES?=$(PSDK_PATH)/out_qnx
VISION_APPS_TARGET_PATH?=$(VISION_APPS_PATH)/out/$(TARGET_SOC)
VISION_APPS_PC_PATH?=$(VISION_APPS_PATH)/out/PC/x86_64/LINUX
HOST_MKIFS=mkifs
BSP_INSTALL_DIR=$(PSDK_QNX_BUILD_PATH)/bsp_800_tda54_vdk/install
QNX_FIRMWARE_PATH=$(BUILT_IMAGES)/rootfs_freertos
MOUNT_POINT?=$(BUILT_IMAGES)/mnt
LOOP_DEV=$(shell losetup -f)
endif # end of ifeq ($(BUILD_QNX_MPU),yes)


psdk_linux_path_check:
ifeq ($(BUILD_LINUX_MPU),yes)
	@if [ ! -f $(PSDK_PATH)/bootfs/bl31.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl31.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/bl32.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl32.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot.img ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot.img not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot-spl.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot-spl.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot-spl ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot-spl not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot-m55-spl ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot-m55-spl not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/tispl.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/tispl.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/Image ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/Image not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/k3-tda54-vdk.dtb ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/k3-tda54-vdk.dtb not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/vmlinux ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/vmlinux not found !!!'; exit 1; fi
else ifeq ($(BUILD_QNX_MPU),yes)
	@if [ ! -f $(PSDK_PATH)/bootfs/bl31.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl31.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/bl32.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/bl32.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot.img ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot.img not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot-spl.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot-spl.bin not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot-spl ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot-spl not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/u-boot-m55-spl ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/u-boot-m55-spl not found !!!'; exit 1; fi
	@if [ ! -f $(PSDK_PATH)/bootfs/tispl.bin ]; then echo 'ERROR: $(PSDK_LINUX_PATH)/board-support/prebuilt-images/tispl.bin not found !!!'; exit 1; fi
endif

vision_apps_path_check:
ifeq ($(BUILD_LINUX_MPU),yes)
	@if [ ! -d $(VISION_APPS_PATH)/out ]; then echo 'ERROR: $(VISION_APPS_PATH)/out not found !!!'; exit 1; fi
endif

vdk: psdk_linux_path_check vision_apps_path_check
ifeq ($(BUILD_LINUX_MPU),yes)
	mkdir -p $(BUILT_IMAGES)
	@echo staging bsps
	cp $(PSDK_PATH)/bootfs/* $(BUILT_IMAGES)

	@echo ""
	@echo staging targetfs
	@echo Processing images at:
	@echo "    $(BUILT_IMAGES)"
	@echo ""
	mkdir -p $(MOUNT_POINT)

	# Create a zero-ed out file of size ~18GB
	dd if=/dev/zero of=${BUILT_IMAGES}/rootfs-img.ext4 bs=1024 count=18874368

	# Emulate this file as a block device and mount it
	sudo umount $(MOUNT_POINT) >/dev/null 2>&1 || true
	sudo losetup $(LOOP_DEV) $(BUILT_IMAGES)/rootfs-img.ext4
	sudo mkfs.ext4 $(LOOP_DEV) >/dev/null 2>&1
	sudo mount $(LOOP_DEV) $(MOUNT_POINT)

	# Actually add contents to the image
	sudo tar -xf $(ROOTFS) -C $(MOUNT_POINT)

	# Add any FileSystem customizations here
	# sudo mkdir -p $MOUNT_POINT/opt
	# sudo touch $MOUNT_POINT/opt/testfile
	#
	# Now the sdcard image is ready, umount and detach loop device
	sudo umount $(MOUNT_POINT)
	sudo losetup -d $(LOOP_DEV)

ifeq ($(BUILD_EMULATION_MODE),yes)
	cp $(VISION_APPS_PC_PATH)/$(BUILD_PROFILE)/libtivision_apps.so.$(PSDK_VERSION) $(BUILT_IMAGES)/libtivision_apps.so
endif

else ifeq ($(BUILD_QNX_MPU),yes)

	rm -rf  $(BUILT_IMAGES)
	mkdir -p $(BUILT_IMAGES)

	# copy application binaries and scripts
	mkdir -p $(QNX_FS_PATH)/vision_apps
	mkdir -p $(QNX_FS_PATH)/tilib
	cp $(VISION_APPS_TARGET_PATH)/$(MPU_CPU)/QNX/$(QNX_APP_PROFILE)/*.out $(QNX_FS_PATH)/vision_apps
	cp $(VISION_APPS_TARGET_PATH)/$(MPU_CPU)/QNX/$(QNX_APP_PROFILE)/libtivision_apps.so.$(PSDK_VERSION) $(QNX_FS_PATH)/tilib
	cp $(PSDK_PATH)/edgeai/QNX/usr/lib/libedgeai-apps-utils.* $(QNX_FS_PATH)/tilib/
	cp $(PSDK_PATH)/edgeai/QNX/usr/lib/libedgeai-tiovx-apps.* $(QNX_FS_PATH)/tilib/
	cp $(PSDK_PATH)/edgeai/QNX/usr/lib/libedgeai-tiovx-kernels.* $(QNX_FS_PATH)/tilib/
	# app_linux_fs_files are not very OS specific. Only input file paths change
	cp -r $(VISION_APPS_PATH)/apps/basic_demos/app_linux_fs_files/vision_apps_all/* $(QNX_FS_PATH)/vision_apps
	cp -r $(VISION_APPS_PATH)/apps/basic_demos/app_linux_fs_files/vision_apps_evm/* $(QNX_FS_PATH)/vision_apps
	-cp -r $(VISION_APPS_PATH)/apps/basic_demos/app_linux_fs_files/*.txt $(QNX_FS_PATH)/vision_apps
	cp $(VISION_APPS_PATH)/apps/basic_demos/app_linux_fs_files/vision_apps_init.sh $(QNX_FS_PATH)/vision_apps/.
	# Rename file paths in app cfg files
	sed -i 's/\/opt\//\/ti_fs\//g' $(QNX_FS_PATH)/vision_apps/*.cfg
	sed -i 's/\/opt\//\/ti_fs\//g' $(QNX_FS_PATH)/vision_apps/app_srv_avp_cfg/*.cfg
	sed -i 's/\/opt\//\/ti_fs\//g' $(QNX_FS_PATH)/vision_apps/*.sh
	# Remove files not needed for QNX
	rm -rf $(QNX_FS_PATH)/vision_apps/limits.conf
	chmod +x $(QNX_FS_PATH)/vision_apps/*.sh

	# copy files to 
	cp -r $(QNX_BOOT_PATH) $(BUILT_IMAGES)
	cp -r $(QNX_FS_PATH) $(BUILT_IMAGES)
	cp -r $(QNX_FS_PATH)/../rootfs_freertos $(BUILT_IMAGES)
	cp -r $(BSP_INSTALL_DIR)/ $(BUILT_IMAGES)/install

	# copy test data
	$(call INSTALL_TEST_DATA,$(BUILT_IMAGES)/qnxfs,vision_apps)

	# copy user.sh script
	cp $(VISION_APPS_PATH)/platform/$(SOC)/qnx/build/user_tda54.sh  /$(BUILT_IMAGES)/qnxfs/scripts/user.sh

ifeq ($(BUILD_EMULATION_MODE),yes)
	cp -f $(VISION_APPS_PC_PATH)/$(QNX_APP_PROFILE)/libtivision_apps.so.$(PSDK_VERSION) $(BUILT_IMAGES)/libtivision_apps.so
endif

ifeq ($(BUILD_CPU_MCU0),yes)
	# copy remote firmware files for mcu0
	$(eval IMAGE_NAME := vx_app_rtos_qnx_mcu0.out)
	cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(QNX_APP_PROFILE)/$(IMAGE_NAME)  $(QNX_FIRMWARE_PATH)/$(QNX_FIRMWARE_PREFIX)-mcu0-fw
endif

ifeq ($(BUILD_CPU_MCU1),yes)
	# copy remote firmware files for mcu1
	$(eval IMAGE_NAME := vx_app_rtos_qnx_mcu1.out)
	cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(QNX_APP_PROFILE)/$(IMAGE_NAME)  $(QNX_FIRMWARE_PATH)/$(QNX_FIRMWARE_PREFIX)-mcu1-fw
endif

ifeq ($(BUILD_CPU_C7x_1),yes)
	# copy remote firmware files for c7x_1
	$(eval IMAGE_NAME := vx_app_rtos_qnx_c7x_1.out)
	cp $(VISION_APPS_TARGET_PATH)/C7604/$(RTOS)/$(QNX_APP_PROFILE)/$(IMAGE_NAME)  $(QNX_FIRMWARE_PATH)/$(QNX_FIRMWARE_PREFIX)-dsp0-fw
endif

ifeq ($(BUILD_CPU_RMCU0_0),yes)
	# copy remote firmware files for rmcu0_0
	$(eval IMAGE_NAME := vx_app_rtos_qnx_rmcu0_0.out)
	cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(QNX_APP_PROFILE)/$(IMAGE_NAME)  $(QNX_FIRMWARE_PATH)/$(QNX_FIRMWARE_PREFIX)-rmcu0_0-fw
endif

	$(HOST_MKIFS) -v -r$(BUILT_IMAGES)/install $(VISION_APPS_PATH)/platform/$(SOC)/qnx/build/tda54_vdk-ti-vision-sdk.build $(BUILT_IMAGES)/bootfs/qnx-ifs
	mv *.sym $(BUILT_IMAGES)/bootfs

	@echo ""
	@echo staging qnxfs
	@echo Processing images at:
	@echo "    $(BUILT_IMAGES)/qnxfs"
	@echo ""
	mkdir -p $(MOUNT_POINT)

	# Create a zero-ed out file of size ~8GB
	dd if=/dev/zero of=${BUILT_IMAGES}/psdk-qnx-emmc.dat bs=1024 count=8388608
	sync
	$(VISION_APPS_PATH)/platform/$(SOC)/qnx/build/qnx_fs.sh ${BUILT_IMAGES}/psdk-qnx-emmc.dat

	# Emulate this file as a block device and mount it
	sudo umount $(MOUNT_POINT) >/dev/null 2>&1 || true
	sudo losetup -f $(BUILT_IMAGES)/psdk-qnx-emmc.dat
	sudo mkfs.vfat $(LOOP_DEV) >/dev/null 2>&1
	sudo mount $(LOOP_DEV) $(MOUNT_POINT)

	# Actually add contents to the image
	sudo cp -r $(BUILT_IMAGES)/qnxfs/* $(MOUNT_POINT)

	# Now the sdcard image is ready, umount and detach loop device
	sudo umount $(MOUNT_POINT)
	sudo losetup -d $(LOOP_DEV)
	rm -rf $(MOUNT_POINT)
endif # end of ifeq ($(BUILD_QNX_MPU),yes)


vdk_copy_c7x:
ifeq ($(BUILD_LINUX_MPU),yes)
	@echo "copying c7x-mma-tidl folder in rootfs"

	sudo umount $(MOUNT_POINT) > /dev/null 2>&1 || true
	sudo mount $(BUILT_IMAGES)/rootfs-img.ext4 $(MOUNT_POINT)

	sudo mkdir -p $(MOUNT_POINT)/opt/c7x
	sudo rm -rf $(MOUNT_POINT)/opt/c7x/*
	sudo cp -r $(PSDK_PATH)/c7x-mma-tidl/*  $(MOUNT_POINT)/opt/c7x/

	sudo umount $(MOUNT_POINT) > /dev/null 2>&1 || true
endif # ifeq ($(BUILD_LINUX_MPU),yes)

vdk_install:
ifeq ($(BUILD_LINUX_MPU),yes)
	@echo "installing updated libs and binaries in rootfs"

	sudo umount $(MOUNT_POINT) > /dev/null 2>&1 || true
	sudo mount $(BUILT_IMAGES)/rootfs-img.ext4 $(MOUNT_POINT)

ifeq ($(BUILD_CPU_MCU0),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_mcu0.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_MCU1),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_mcu1.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_MCU2),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_mcu2.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_MCU3),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_mcu3.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_MCU4),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/M55/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_mcu4.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_RMCU0_0),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_rmcu0_0.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_RMCU0_1),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_rmcu0_1.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_RMCU1_0),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_rmcu1_0.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_RMCU1_1),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_rmcu1_1.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_RMCU2_0),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_rmcu2_0.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_RMCU2_1),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/R52P/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_rmcu2_1.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/C7604/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_c7x_1.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
	sudo cp $(VISION_APPS_TARGET_PATH)/C7604/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_c7x_1.out $(BUILT_IMAGES)
endif
ifeq ($(BUILD_CPU_C7x_2),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/C7604/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_c7x_2.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_C7x_3),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/C7604/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_c7x_3.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif
ifeq ($(BUILD_CPU_C7x_4),yes)
	sudo cp $(VISION_APPS_TARGET_PATH)/C7604/$(RTOS)/$(BUILD_PROFILE)/vx_app_rtos_linux_c7x_4.out $(MOUNT_POINT)/lib/firmware/vision_apps_evm
endif

	sudo cp $(VISION_APPS_TARGET_PATH)/A720/LINUX/$(BUILD_PROFILE)/libtivision_apps.so $(MOUNT_POINT)/usr/lib
	sudo cp $(VISION_APPS_TARGET_PATH)/A720/LINUX/$(BUILD_PROFILE)/libtivision_apps.so.$(PSDK_VERSION) $(MOUNT_POINT)/usr/lib

ifeq ($(BUILD_EMULATION_MODE),yes)
	sudo cp $(VISION_APPS_PC_PATH)/$(BUILD_PROFILE)/libtivision_apps.so.$(PSDK_VERSION) $(BUILT_IMAGES)/libtivision_apps.so
endif

	sudo cp $(VISION_APPS_TARGET_PATH)/A720/LINUX/$(BUILD_PROFILE)/vx_app_* $(MOUNT_POINT)/opt/vision_apps
	sudo cp -r $(VISION_APPS_PATH)/apps/basic_demos/app_linux_fs_files/vision_apps_evm/* $(MOUNT_POINT)/opt/vision_apps

	sudo umount $(MOUNT_POINT) > /dev/null 2>&1 || true

endif # ifeq ($(BUILD_LINUX_MPU),yes)

vdk_rootfs_scrub:
ifeq ($(BUILD_LINUX_MPU),yes)
	@echo "cleaning up rootfs"
	rm -rf $(BUILT_IMAGES)/rootfs-img.ext4
endif # ifeq ($(BUILD_LINUX_MPU),yes)

vdk_prebuilt_scrub:
ifeq ($(BUILD_LINUX_MPU),yes)
	@echo "cleaning up prebuilt"
	rm -rf $(BUILT_IMAGES)/u-boot*
	rm -rf $(BUILT_IMAGES)/bl31*
	rm -rf $(BUILT_IMAGES)/bl32*
	rm -rf $(BUILT_IMAGES)/Image
	rm -rf $(BUILT_IMAGES)/vmlinux
	rm -rf $(BUILT_IMAGES)/*.dtb
	rm -rf $(BUILT_IMAGES)/tispl.bin
	rm -rf $(BUILT_IMAGES)/vx_*
	rm -rf $(BUILT_IMAGES)/libtivision_apps.so.*
endif # ifeq ($(BUILD_LINUX_MPU),yes)

vdk_scrub_all:
	@echo "cleaning up stage path"
	rm -rf $(BUILT_IMAGES)
