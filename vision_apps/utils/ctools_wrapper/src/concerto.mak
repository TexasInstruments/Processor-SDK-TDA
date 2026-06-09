ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX QNX))
ifeq ($(CTOOLS_BUILD_ENABLED), yes)

include $(PRELUDE)

TARGET      := app_utils_ctools_wrapper
TARGETTYPE  := library

CSOURCES    := ctools_wrapper.c

# Include paths
IDIRS += $(VISION_APPS_PATH)/utils/ctools_wrapper/include
IDIRS += $(APP_UTILS_PATH)/utils/ctools/include
IDIRS += $(APP_UTILS_PATH)/utils/ctools/include/soc/$(SOC)
IDIRS += $(VISION_APPS_PATH)

include $(FINALE)

endif
endif

