ifneq ($(TARGET_PLATFORM), PC)

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))

        include $(PRELUDE)

        TARGET      := app_utils_codec_wrapper
        TARGETTYPE  := library

        ifeq ($(TARGET_OS),LINUX)
            CSOURCES    := codec_wrapper_linux.c

            IDIRS += $(LINUX_FS_PATH)/usr/include/gstreamer-1.0/
            IDIRS += $(LINUX_FS_PATH)/usr/include/glib-2.0/
            IDIRS += $(LINUX_FS_PATH)/usr/lib/glib-2.0/include/

            STATIC_LIBS += app_utils_gst_wrapper
        endif

        ifeq ($(TARGET_OS),QNX)
            CSOURCES    := codec_wrapper_qnx.c

            ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721E))
                IDIRS += $(PSDK_QNX_PATH)/qnx/codec/img/qnx/OpenMAXIL/khronos/openmaxil/
            endif

            ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J784S4 J742S2 J721S2 J722S AM62A))
                IDIRS += $(PSDK_QNX_PATH)/qnx/codec/vpu/OpenMAXIL/khronos/openmaxil/
            endif

            STATIC_LIBS += app_utils_omax_wrapper
        endif

        include $(FINALE)
    endif
endif
