ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)

    TARGET      := app_utils_cfg_parser
    TARGETTYPE  := library

    CFLAGS      += -Wno-parentheses -Wno-sign-compare

    CPPSOURCES  := app_cfg_parser.cpp

    IDIRS       += $(VISION_APPS_PATH)/utils/cfg_parser/include
    IDIRS       += $(TIOVX_PATH)/include

    include $(FINALE)

endif
