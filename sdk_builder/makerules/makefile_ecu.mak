# ECU_BUILD flag is used to enable ECU build (power optimization) for vision apps.
# ECU build is only supported on J722S, j721s2 and J784S4 platforms with QNX only and not available for Linux.
# Set to demo name to enable ECU build. Default is 'no' to disable ECU Build for backward compatibility.
# ECU_BUILD options for J722S   : fc
# ECU_BUILD options for J721S2  : srv, fc, avp4
# ECU_BUILD options for J784S4  : srv, fc, avp4

ECU_BUILD?=no

# Check if ECU_BUILD is set to a valid values
ifneq ($(ECU_BUILD), no)
    ifeq ($(BUILD_LINUX_MPU),yes)
        $(error "ECU build is not supported on Linux, Only enabled for QNX. Set BUILD_LINUX_MPU to 'no' and and BUILD_QNX_MPU to 'yes' to enable ECU build.")
    endif
    ifeq ($(SOC), j722s)
        ifneq ($(ECU_BUILD), fc)
            $(error "Invalid ECU_BUILD value: $(ECU_BUILD) for SOC=$(SOC). Must be 'no' or 'fc'.")
        endif
    else ifeq ($(SOC),$(filter $(SOC), j721s2 j784s4 j742s2))
        ifneq ($(filter $(ECU_BUILD), srv fc avp4), $(ECU_BUILD))
            $(error "Invalid ECU_BUILD value: $(ECU_BUILD) for SOC=$(SOC). Must be one of 'no', 'srv', 'fc', or 'avp4'.")
        endif
    else
        $(error "Unsupported SOC: $(SOC). ECU build is only supported on J722S, J721S2 and J784S4 platforms.")
    endif
    # Set macro and pass to the compiler
    # Create BUILD_DEFS flag (ECU_SRV, ECU_FC or ECU_AVP4) if ECU_BUILD is set to a valid value
    ECU_BUILD_DEF ?= ECU_$(shell echo $(ECU_BUILD) | tr a-z A-Z)
endif

# Set the auto start command for ECU build
# ECU_AUTO_START_CMD is set based on the ECU_BUILD value.
ifneq ($(ECU_BUILD), no) 
    ifeq ($(ECU_BUILD),srv)
        ECU_AUTO_START_CMD=/ti_fs/vision_apps/auto_run_app_srv.sh
    else ifeq ($(ECU_BUILD),fc)
        ECU_AUTO_START_CMD=/ti_fs/vision_apps/auto_run_app_tidl_front_cam.sh
    else ifeq ($(ECU_BUILD),avp4)
        ECU_AUTO_START_CMD=/ti_fs/vision_apps/auto_run_app_tidl_avp4.sh
    else
        $(error "Invalid ECU_BUILD value: $(ECU_BUILD). Must be one of 'srv', 'fc', or 'avp4'.")
    endif
endif

# SET_AUTORUN_ECU_TO_QNX_SD_FS macro add the autorun command on the QNX SD filesystem based on the ECU_BUILD value.
# It assumes that the QNX SD filesystem is mounted at $(QNX_SD_FS_QNX_PATH).
define SET_AUTORUN_ECU_TO_QNX_SD_FS =
	@if [ "$(ECU_BUILD)" != "no" ]; then \
		echo 'Installing $(ECU_BUILD) ECU auto start command to QNX SD filesystem...'; \
		echo $(ECU_AUTO_START_CMD) >> $(QNX_SD_FS_QNX_PATH)/scripts/user.sh; \
		sync; \
		echo 'ECU auto start command installed to QNX SD filesystem.'; \
	fi
endef
