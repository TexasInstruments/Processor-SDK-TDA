
include build_flags.mak
include tools_path.mak

# Edit below file to change default build options
include platform_build_flags.mak

# Additional make targets to build various related components
ifeq ($(RTOS_SDK),pdk)
    include makerules/makefile_pdk.mak
else ifeq ($(RTOS_SDK),mcu_sdk)
    include makerules/makefile_mcu_sdk.mak
    include makerules/makefile_dmautils.mak
else
    include makerules/makefile_mcu_plus_sdk.mak
endif

include makerules/makefile_tidl_mmalib.mak
include makerules/makefile_tiovx_ptk_imaging.mak
include makerules/makefile_platform.mak
include makerules/makefile_test_data.mak
include makerules/makefile_linux_arm.mak
include makerules/makefile_help.mak
include makerules/makefile_config.mak
include makerules/makefile_tiadalg.mak
include makerules/makefile_check_paths.mak
include makerules/makefile_qnx.mak
ifeq ($(RTOS_SDK),pdk)
include makerules/makefile_sbl.mak
include makerules/makefile_ethfw.mak
endif
include makerules/makefile_ipk.mak
include makerules/makefile_uboot.mak
include makerules/makefile_sbl_combined_appimage.mak
include makerules/makefile_app_kernels.mak
include makerules/makefile_vision_apps.mak
include makerules/makefile_edgeai.mak
include makerules/makefile_vdk.mak

# Set Variables for SOC specific rule dependencies used in make rules below
ifeq ($(SOC), $(filter $(SOC), j721e j721s2 j784s4 j742s2))
    SOC_VARIABLE_RULES=tiadalg qnx
    SOC_VARIABLE_CLEAN=tiadalg_clean qnx_clean sbl_clean
    SOC_VARIABLE_SCRUB=tiadalg_scrub qnx_scrub sbl_scrub
    SOC_VARIABLE_DOCS=tiadalg_docs
    ifeq ($(BUILD_ENABLE_ETHFW),yes)
        SOC_VARIABLE_RULES += ethfw
        SOC_VARIABLE_CLEAN += ethfw_clean
        SOC_VARIABLE_SCRUB += ethfw_scrub
    endif
else ifeq ($(SOC), $(filter $(SOC), j722s))
    SOC_VARIABLE_RULES=qnx
    SOC_VARIABLE_CLEAN=qnx_clean sbl_clean
    SOC_VARIABLE_SCRUB=qnx_scrub sbl_scrub
    SOC_VARIABLE_DOCS=
else ifeq ($(SOC), $(filter $(SOC), am62a))
    SOC_VARIABLE_RULES=qnx
    SOC_VARIABLE_CLEAN=qnx_clean sbl_clean
    SOC_VARIABLE_SCRUB=qnx_scrub sbl_scrub
    SOC_VARIABLE_DOCS=
else ifeq ($(SOC), $(filter $(SOC), tda54))
    SOC_VARIABLE_RULES=qnx dmautils
    SOC_VARIABLE_CLEAN=qnx_clean dmautils_clean
    SOC_VARIABLE_SCRUB=qnx_scrub dmautils_scrub
    SOC_VARIABLE_DOCS=
endif

ifeq ($(BUILD_PTK),yes)
    SOC_VARIABLE_RULES += ptk
    SOC_VARIABLE_CLEAN += ptk_clean
    SOC_VARIABLE_SCRUB += ptk_scrub
    SOC_VARIABLE_DOCS  += ptk_docs
endif

sdk: sdk_check_paths rtos_sdk
	$(MAKE) app_utils
	$(MAKE) vxlib
	$(MAKE) tiovx
	$(MAKE) platform_hlos
	$(MAKE) imaging
	$(MAKE) video_io
	$(MAKE) $(SOC_VARIABLE_RULES)
	$(MAKE) tidl_lib
	$(MAKE) tidl_tiovx_kernels
ifeq ($(BUILD_ENABLE_TIDL_RT),yes)
	$(MAKE) tidl_rt
endif
	$(MAKE) app_kernels
	$(MAKE) vision_apps
	$(MAKE) platform_rtos
ifeq ($(BUILD_CPU_MCU1_0),yes)
	$(MAKE) uboot
endif

sdk_clean: sdk_check_paths rtos_sdk_clean app_utils_clean imaging_clean video_io_clean vxlib_clean tiovx_clean tidl_lib_clean tidl_tiovx_kernels_clean tidl_rt_clean app_kernels_clean platform_clean vision_apps_clean $(SOC_VARIABLE_CLEAN)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	$(MAKE) uboot_clean
endif

sdk_scrub: sdk_check_paths rtos_sdk_scrub app_utils_scrub imaging_scrub video_io_scrub vxlib_scrub tiovx_scrub tidl_lib_scrub tidl_tiovx_kernels_scrub tidl_rt_scrub app_kernels_scrub platform_scrub vision_apps_scrub $(SOC_VARIABLE_SCRUB)
ifeq ($(BUILD_CPU_MCU1_0),yes)
	$(MAKE) uboot_clean
endif

sdk_docs: sdk_check_paths tiovx_docs vision_apps_docs $(SOC_VARIABLE_DOCS)
	$(MAKE) -C $(PSDK_PATH)/psdk_rtos sphinx_docs DEVFAMILY=$(TARGET_SOC)

#KW build: Split the build into two - components which need not be part of
# SDK KW report as the report for these components are separately delivered
# Build it first and the use "kw_build" target for KW inject
kw_pre_build: rtos_sdk

kw_build: sdk

#RTOS build: invokes pdk, mcu_plus_sdk, or mcu_sdk

ifeq ($(RTOS_SDK),pdk)
    rtos_sdk:pdk vhwa
    rtos_sdk_clean:pdk_clean vhwa_clean
    rtos_sdk_scrub:pdk_scrub vhwa_clean
else ifeq ($(RTOS_SDK),mcu_sdk)
    rtos_sdk:mcu_sdk
    rtos_sdk_clean:mcu_sdk_clean
    rtos_sdk_scrub:mcu_sdk_scrub
else
    rtos_sdk:mcu_plus_sdk
    rtos_sdk_clean:mcu_plus_sdk_clean
    rtos_sdk_scrub:mcu_plus_sdk_scrub
endif
