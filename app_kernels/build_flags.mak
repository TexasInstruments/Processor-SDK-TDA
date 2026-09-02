PSDK_PATH ?= $(abspath ..)
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder

include $(PSDK_BUILDER_PATH)/build_flags.mak
include $(PSDK_BUILDER_PATH)/platform_build_flags.mak
