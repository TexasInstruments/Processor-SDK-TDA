ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720 R5F M55))

    include $(PRELUDE)
    TARGET      := app_utils_draw2d
    TARGETTYPE  := library

    CSOURCES    := $(call all-c-files)

    IDIRS       := $(VISION_APPS_PATH)

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), R5F))
        DEFS+=EXCLUDE_BMP_LOAD
    endif

    include $(FINALE)

endif
