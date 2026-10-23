#
# Utility makefile to show important make targets
#

sdk_help:
	#
	# Use below make targets to build SDK and/or different components with the SDK
	#
	# make tidl                  # Build TIDL (needs TIDL source package)
	# make tidl_clean            # Clean TIDL (needs TIDL source package)
	#
	# make ptk                   # Build PTK (needs PTK source package)
	# make ptk_clean             # Clean PTK (needs PTK source package)
	# make ptk_scrub             # Delete all generated files and folders in PTK (needs PTK source package)
	#
	# make pdk                   # Build PDK, also builds SBL
	# make pdk_clean             # Clean PDK, also cleans SBL
	# make pdk_scrub             # Delete all generated files and folders in PDK including SBL
	#
	# make tiovx                 # Build TIOVX
	# make tiovx_clean           # Clean TIOVX
	# make tiovx_scrub           # Delete all generated files and folders in TIOVX
	#
	# make imaging               # Build IMAGING
	# make imaging_clean         # Clean IMAGING
	# make imaging_scrub         # Delete all generated files and folders in IMAGING
	#
	# make                       # Build vision apps
	# make vision_apps           # Build vision apps
	# make vision_apps_clean     # Clean vision apps
	# make vision_apps_scrub     # Delete all generated files and folders in vision apps
	#
	# make qnx                                         # Build QNX related drivers and libraries
	# make qnx_fs_install                              # Install application binaries, bootloader
	#                                                    to local host QNX target filesystem
	# make qnx_fs_install_sd                           # Copy application images from local host filesystem to SD card filesystem
	#                                                    AND copy SPL/uboot files (QNX)
	# make qnx_fs_install_sd_sbl                       # Copy application images from local host filesystem to SD card filesystem
	#                                                    AND copy SBL/MCUSW_boot_app files (QNX)
	# make qnx_fs_install_sd_test_data                 # Copy test data required for running the applications
	#                                                    to SD card filesystem. DOES NOT invoke 'qnx_fs_install_sd'
	#
	# make linux_fs_install                            # Install application binaries, dtb files, bootloader
	#                                                    and kernel images to local host linux target filesystem
	# make linux_fs_install_sd                         # Copy application images from local host filesystem to SD card filesystem
	#                                                    This also invokes 'linux_fs_install' to do a local copy before copying to SD card
	# make linux_fs_install_sd_test_data               # Copy test data required for running the applications
	#                                                    to SD card filesystem. DOES NOT invoke 'linux_fs_install_sd'
	# make linux_fs_install_nfs                        # Copy application images to local host filesystem for NFS boot
	#                                                    This also invokes 'linux_fs_install' to do a local copy
	# make linux_fs_install_nfs_test_data              # Copy test data required for running the applications using NFS boot
	#                                                    to SD card filesystem.
	#
	# make sdk_show_config       # Show SDK build variables
	# make sdk_check_paths       # Check if all paths needed by vision apps exists
	# make sdk_help              # Show supported makefile target's
	#
	# make sdk                   # Build SDK
	# make sdk_clean             # Clean SDK
	# make sdk_scrub             # Delete all generated files and folders in SDK
	#

