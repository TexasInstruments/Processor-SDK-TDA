
PSDK_PATH ?= $(abspath ..)
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder

# paths for components shared between tiovx and vision_apps are specified in below
# file in tiovx, ex, bios, tidl, pdk, cgtools, ...
include $(PSDK_BUILDER_PATH)/tools_path.mak
