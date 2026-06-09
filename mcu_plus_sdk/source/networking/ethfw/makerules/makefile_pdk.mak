#
# Utility makefile to build PDK libaries and related components
#
# Edit this file to suit your specific build needs
#

MAKE_EXTRA_OPTIONS ?= -j $(HOST_NUM_CORES)

ifeq ($(PROFILE), $(filter $(PROFILE),release all))
  PDK_BUILD_PROFILE_LIST_ALL+=release
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
  PDK_BUILD_PROFILE_LIST_ALL+=debug
endif

ifeq ($(BUILD_CPU_MPU1),yes)
  PDK_CORE_LIST_ALL+=mpu1_0
endif
ifeq ($(BUILD_CPU_MCU1_0),yes)
  PDK_CORE_LIST_ALL+=mcu1_0
endif
ifeq ($(BUILD_CPU_MCU1_1),yes)
  PDK_CORE_LIST_ALL+=mcu1_1
endif
ifeq ($(BUILD_CPU_MCU2_0),yes)
  PDK_CORE_LIST_ALL+=mcu2_0
endif
ifeq ($(BUILD_CPU_MCU2_1_SERVER),yes)
  PDK_CORE_LIST_ALL+=mcu2_1
endif
ifeq ($(BUILD_CPU_MCU2_1),yes)
  PDK_CORE_LIST_ALL+=mcu2_1
endif
ifeq ($(BUILD_CPU_MCU3_0),yes)
  PDK_CORE_LIST_ALL+=mcu3_0
endif
ifeq ($(BUILD_CPU_MCU3_1),yes)
  PDK_CORE_LIST_ALL+=mcu3_1
endif
ifeq ($(BUILD_CPU_C6x_1),yes)
  PDK_CORE_LIST_ALL+=c66xdsp_1
endif
ifeq ($(BUILD_CPU_C6x_2),yes)
  PDK_CORE_LIST_ALL+=c66xdsp_2
endif
ifeq ($(BUILD_CPU_C7x_1),yes)
  PDK_CORE_LIST_ALL+=c7x
endif

pdk_build:
	$(MAKE) -C $(PDK_PATH)/packages/ti/build BOARD=${PDK_BOARD} custom_target BUILD_PROFILE_LIST_ALL="$(sort ${PDK_BUILD_PROFILE_LIST_ALL})" CORE_LIST_ALL="$(sort ${PDK_CORE_LIST_ALL})" BUILD_TARGET_LIST_ALL="$(sort ${PDK_BUILD_TARGET_LIST_ALL})" -s $(MAKE_EXTRA_OPTIONS) PDK_INSTALL_PATH:=${PDK_PATH}/packages SDK_INSTALL_PATH:=$(PSDK_PATH) TOOLS_INSTALL_PATH:=$(PSDK_TOOLS_PATH)

pdk:
	$(foreach soc, $(sort ${SOC_LIST}),\
	$(MAKE) pdk_build PDK_BOARD=${${soc}_BOARD} PDK_BUILD_TARGET_LIST_ALL="pdk_libs pdk_app_libs" &&\
	) $(NOP)

pdk_clean:
	$(foreach soc, $(sort ${SOC_LIST}),\
	$(MAKE) pdk_build PDK_BOARD=${${soc}_BOARD} PDK_BUILD_TARGET_LIST_ALL="pdk_libs_clean pdk_app_libs_clean" &&\
	) $(NOP)

.SILENT:pdk_custom_libs pdk_custom_libs_clean

pdk_custom_libs:
	$(foreach profile, $(sort ${PDK_BUILD_PROFILE_LIST_ALL}),\
	$(foreach soc, $(sort ${PDK_SOC_LIST}),\
	$(foreach rule, $(sort ${PDK_LIB_RULES}),\
	$(foreach core, $(filter ${$(soc)_CORE_LIST},$(sort ${PDK_CORE_LIST_ALL})), \
	$(MAKE) -C $(PDK_PATH)/packages/ti/build  ${rule} CORE=${core} BUILD_PROFILE=${profile} BOARD=${$(soc)_BOARD} PDK_INSTALL_PATH:=${PDK_PATH}/packages SDK_INSTALL_PATH:=$(PSDK_PATH) &&\
	) \
	) \
	) \
	) $(NOP)

pdk_custom_libs_clean:
	$(foreach profile, $(sort ${PDK_BUILD_PROFILE_LIST_ALL}),\
	$(foreach soc, $(sort ${PDK_SOC_LIST}),\
	$(foreach rule, $(sort ${PDK_LIB_RULES}),\
	$(foreach core, $(filter ${$(soc)_CORE_LIST},$(sort ${PDK_CORE_LIST_ALL})), \
	$(MAKE) -C $(PDK_PATH)/packages/ti/build  $(patsubst %,%_clean,${rule}) CORE=${core} BUILD_PROFILE=${profile} BOARD=${$(soc)_BOARD} PDK_INSTALL_PATH:=${PDK_PATH}/packages SDK_INSTALL_PATH:=$(PSDK_PATH) &&\
	) \
	) \
	) \
	) $(NOP)

.PHONY: pdk pdk_build pdk_clean pdk_custom_libs pdk_custom_libs_clean
