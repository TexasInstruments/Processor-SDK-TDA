#
# Utility makefile to build TIDL libaries and related components
#
# Edit this file to suit your specific build needs
#

export MMALIB_PATH
export PSDK_INSTALL_PATH=$(PSDK_PATH)

include $(CONCERTO_ROOT)/shell.mak

NUM_THREADS := $(shell nproc --ignore=2)

mmalib:
ifeq ($(BUILD_TARGET_MODE),yes)
	$(MAKE) -C $(MMALIB_PATH) mmalib mmalib_cn common SRC_DIR=cnn TARGET_BUILD=release
endif
ifeq ($(BUILD_EMULATION_MODE),yes)
	$(MAKE) -C $(MMALIB_PATH) mmalib mmalib_cn common SRC_DIR=cnn TARGET_CPU=x86_64 TARGET_SCPU=$(C7X_TARGET) TARGET_PLATFORM=PC TARGET_BUILD=release
endif

mmalib_clean mmalib_scrub:
	rm -rf $(MMALIB_PATH)/out

tidl_lib:
ifeq ($(BUILD_EMULATION_MODE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH) tidl_lib DSP_TOOLS=$(CGT7X_ROOT) TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
    )
endif
ifeq ($(BUILD_TARGET_MODE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH) tidl_lib DSP_TOOLS=$(CGT7X_ROOT) TARGET_PLATFORM=TI_DEVICE TARGET_BUILD=$(current_profile); \
    )
endif

tidl_tiovx_kernels:
ifeq ($(BUILD_EMULATION_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels TARGET_BUILD=$(current_profile); \
	)
endif
endif

tidl_rt:
ifeq ($(BUILD_EMULATION_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/tiovx_kernels all TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH)/arm-tidl all TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl all TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(BUILD_LINUX_MPU),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl all TARGET_BUILD=$(current_profile); \
    )
endif
endif

tidl_pc_tools:
ifeq ($(BUILD_EMULATION_MODE),yes)
	$(MAKE) BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no PROFILE=release rtos_sdk -j${NUM_THREADS}
	$(MAKE) BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no PROFILE=release app_utils -j${NUM_THREADS}
	$(MAKE) BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no BUILD_HWA_KERNELS=no PROFILE=release tiovx -j${NUM_THREADS}
	$(MAKE) -C $(TIDL_PATH) TARGET_PLATFORM=PC -j${NUM_THREADS}
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels TARGET_PLATFORM=PC -j${NUM_THREADS}
	$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels TARGET_PLATFORM=PC -j${NUM_THREADS}
else
	$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels TARGET_PLATFORM=PC -j${NUM_THREADS}
endif
	$(MAKE) -C $(TIDL_PATH)/arm-tidl all TARGET_PLATFORM=PC -j${NUM_THREADS}
	rm -rf $(PSDK_PATH)/tidl_tools.tar.gz
	cd $(TIDL_PATH) && tar -h -czf tidl_tools.tar.gz tidl_tools/
	mv $(TIDL_PATH)/tidl_tools.tar.gz $(PSDK_PATH)/
endif

tidl_lib_scrub: tidl_lib_clean
	rm -rf $(TIDL_PATH)/out

tidl_lib_clean:
ifeq ($(BUILD_EMULATION_MODE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH) tidl_lib_clean DSP_TOOLS=$(CGT7X_ROOT) TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
    )
endif
ifeq ($(BUILD_TARGET_MODE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH) tidl_lib_clean DSP_TOOLS=$(CGT7X_ROOT) TARGET_PLATFORM=TI_DEVICE TARGET_BUILD=$(current_profile); \
    )
endif

tidl_tiovx_kernels_scrub:
ifeq ($(BUILD_EMULATION_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels_scrub TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels_clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels_scrub TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels_scrub TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels_clean TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels_scrub TARGET_BUILD=$(current_profile); \
	)
endif
endif

tidl_tiovx_kernels_clean:
ifeq ($(BUILD_EMULATION_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels_clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels_clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels_clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_arm_kernels_clean TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH) tidl_tiovx_dsp_kernels_clean TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl tidl_tiovx_kernels_clean TARGET_BUILD=$(current_profile); \
	)
endif
endif

tidl_rt_scrub:
ifeq ($(BUILD_EMULATION_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/tiovx_kernels scrub TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH)/arm-tidl scrub TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl scrub TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(BUILD_LINUX_MPU),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl scrub TARGET_BUILD=$(current_profile); \
    )
endif
endif

tidl_rt_clean:
ifeq ($(BUILD_EMULATION_MODE),yes)
ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/tiovx_kernels clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
		$(MAKE) -C $(TIDL_PATH)/arm-tidl clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
else
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl clean TARGET_PLATFORM=PC TARGET_BUILD=$(current_profile); \
	)
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(BUILD_LINUX_MPU),yes)
	$(foreach current_profile, $(BUILD_PROFILE_LIST_ALL),\
		$(MAKE) -C $(TIDL_PATH)/arm-tidl clean TARGET_BUILD=$(current_profile); \
    )
endif
endif

.PHONY: tidl_lib tidl_lib_clean tidl_lib_scrub tidl_tiovx_kernels tidl_tiovx_kernels_clean tidl_tiovx_kernels_scrub tidl_rt tidl_rt_clean tidl_rt_scrub mmalib mmalib_clean mmalib_scrub
