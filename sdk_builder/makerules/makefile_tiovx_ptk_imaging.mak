#
# Utility makefile to build TIOVX, PTK and imaging libaries and related components
#
# Edit this file to suit your specific build needs
#

app_utils:
	$(MAKE) -C $(APP_UTILS_PATH)

app_utils_clean:
	$(MAKE) -C $(APP_UTILS_PATH) clean

app_utils_scrub:
	$(MAKE) -C $(APP_UTILS_PATH) scrub

vxlib:
ifeq ($(BUILD_EMULATION_MODE),yesDISABLE)
ifeq ($(PROFILE), $(filter $(PROFILE),release all))
	TARGET_PLATFORM=PC TARGET_CPU=x86_64 TARGET_SCPU=C66 TARGET_BUILD=release $(MAKE) -C $(VXLIB_PATH) vxlib
	TARGET_PLATFORM=PC TARGET_CPU=x86_64 TARGET_SCPU=C66 TARGET_BUILD=release $(MAKE) -C $(VXLIB_PATH) cp_to_lib
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
	TARGET_PLATFORM=PC TARGET_CPU=x86_64 TARGET_SCPU=C66 TARGET_BUILD=debug $(MAKE) -C $(VXLIB_PATH) vxlib
	TARGET_PLATFORM=PC TARGET_CPU=x86_64 TARGET_SCPU=C66 TARGET_BUILD=debug $(MAKE) -C $(VXLIB_PATH) cp_to_lib
endif
endif
ifeq ($(BUILD_TARGET_MODE),yes)
ifeq ($(SOC),j721e)
ifeq ($(PROFILE), $(filter $(PROFILE),release all))
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=C66 TARGET_BUILD=release $(MAKE) -C $(VXLIB_PATH) vxlib
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=C66 TARGET_BUILD=release $(MAKE) -C $(VXLIB_PATH) cp_to_lib
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=C66 TARGET_BUILD=debug $(MAKE) -C $(VXLIB_PATH) vxlib
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=C66 TARGET_BUILD=debug $(MAKE) -C $(VXLIB_PATH) cp_to_lib
endif
endif
ifeq ($(SOC), $(filter $(SOC),j721s2 j784s4 j742s2 am62a j722s tda54))
ifeq ($(PROFILE), $(filter $(PROFILE),release all))
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=$(C7X_TARGET) TARGET_BUILD=release $(MAKE) -C $(VXLIB_PATH) vxlib
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=$(C7X_TARGET) TARGET_BUILD=release $(MAKE) -C $(VXLIB_PATH) cp_to_lib
endif
ifeq ($(PROFILE), $(filter $(PROFILE),debug all))
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=$(C7X_TARGET) TARGET_BUILD=debug $(MAKE) -C $(VXLIB_PATH) vxlib
	TARGET_PLATFORM=$(TARGET_SOC) TARGET_CPU=$(C7X_TARGET) TARGET_BUILD=debug $(MAKE) -C $(VXLIB_PATH) cp_to_lib
endif
endif
endif

vxlib_scrub:
	$(MAKE) -C $(VXLIB_PATH) scrub

vxlib_clean:
	$(MAKE) -C $(VXLIB_PATH) clean

tiovx:
	$(MAKE) -C $(TIOVX_PATH) RTOS_SDK=$(RTOS_SDK) vision_apps_utils
	$(MAKE) -C $(TIOVX_PATH) RTOS_SDK=$(RTOS_SDK)

tiovx_clean:
	$(MAKE) -C $(TIOVX_PATH) clean

tiovx_scrub:
	$(MAKE) -C $(TIOVX_PATH) scrub

tiovx_docs:
	$(MAKE) -C $(TIOVX_PATH) doxy_docs

video_io:
	$(MAKE) -C $(VIDEO_IO_PATH) RTOS_SDK=$(RTOS_SDK)

video_io_clean:
	$(MAKE) -C $(VIDEO_IO_PATH) clean

video_io_scrub:
	$(MAKE) -C $(VIDEO_IO_PATH) scrub

imaging:
	$(MAKE) -C $(IMAGING_PATH)

imaging_clean:
	$(MAKE) -C $(IMAGING_PATH) clean

imaging_scrub:
	$(MAKE) -C $(IMAGING_PATH) scrub

ifneq ($(SOC), $(filter $(SOC), tda54))
ptk:
ifeq ($(BUILD_PTK),yes)
ifeq ($(BUILD_EMULATION_MODE),yes)
	-$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) BUILD_TARGET_MODE=no
	-$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) cp_to_lib BUILD_TARGET_MODE=no
endif
ifeq ($(BUILD_TARGET_MODE),yes)
	-$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) BUILD_TARGET_MODE=yes
	-$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) cp_to_lib BUILD_TARGET_MODE=yes
endif
endif

ptk_clean:
ifeq ($(BUILD_PTK),yes)
	-$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) clean
endif

ptk_docs:
ifeq ($(BUILD_PTK),yes)
	$(MAKE) -C $(PTK_PATH) doxy_docs
endif

ptk_scrub:
ifeq ($(BUILD_PTK),yes)
ifeq ($(BUILD_EMULATION_MODE),yes)
	$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) BUILD_TARGET_MODE=no scrub
endif
ifeq ($(BUILD_TARGET_MODE),yes)
	$(MAKE) -C $(PTK_PATH) TARGET_SOC=$(TARGET_SOC) RTOS=$(RTOS) C7X_TARGET=$(C7X_TARGET) BUILD_TARGET_MODE=yes scrub
endif
endif

.PHONY: tiovx tiovx_clean ptk ptk_clean imaging imaging_clean video_io video_io_clean video_io_scrub app_utils app_utils_clean app_utils_scrub
endif
