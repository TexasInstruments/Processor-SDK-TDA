PSDK_PATH ?= $(abspath ..)
PSDK_BUILDER_PATH ?= $(PSDK_PATH)/sdk_builder

include $(PSDK_PATH)/tiovx/build_flags.mak
include $(PSDK_PATH)/imaging/build_flags.mak
include $(PSDK_BUILDER_PATH)/build_flags.mak
include $(PSDK_BUILDER_PATH)/platform_build_flags.mak
include $(PSDK_BUILDER_PATH)/makerules/makefile_ecu.mak

# Used to version control the tivision_apps.so
ifeq ($(SOC_FAMILY), SOC_FAMILY_TDA5)
    PSDK_VERSION?=12.1.0
else ifeq ($(SOC_FAMILY), $(filter $(SOC_FAMILY), SOC_FAMILY_AM SOC_FAMILY_J7))
    PSDK_VERSION?=11.2.1
endif

# Set to 'yes' to link all .out files against libtivision_apps.so instead of static libs
LINK_SHARED_OBJ?=yes
ifeq ($(SOC),$(filter $(SOC), j722s tda54))
    # temporarily setting this to "no" for the QNX J722S and TDA54 until the full BSP is received which
    # has the screen package
    ifeq ($(BUILD_QNX_MPU),yes)
        LINK_SHARED_OBJ=no
    endif
endif