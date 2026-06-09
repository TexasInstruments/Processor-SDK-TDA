# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)
    ifeq ($(BUILD_VIDEO_IO_KERNELS),yes)
        LDIRS       += $(VIDEO_IO_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        STATIC_LIBS += vx_kernels_video_io_tests vx_kernels_video_io
    endif
endif #ifeq ($(TARGET_CPU),x86_64)
