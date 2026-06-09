ifneq ($(TARGET_PLATFORM),PC)
    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX))

    include $(PRELUDE)

    TARGET      := app_utils_gst_wrapper
    TARGETTYPE  := library

    CSOURCES    := gst_wrapper.c

    IDIRS += $(LINUX_FS_PATH)/usr/include/gstreamer-1.0/
    IDIRS += $(LINUX_FS_PATH)/usr/include/glib-2.0/
    IDIRS += $(LINUX_FS_PATH)/usr/lib/glib-2.0/include/

    include $(FINALE)

    endif
endif
