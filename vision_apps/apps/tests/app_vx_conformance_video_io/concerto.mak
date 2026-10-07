ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)

    TARGET      := vx_app_conformance_video_io
    TARGETTYPE  := exe
    CSOURCES    := $(call all-c-files)

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
        include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
    endif

    ifeq ($(TARGET_CPU),x86_64)
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
    endif

    #IDIRS += ${TIOVX_PATH}
    IDIRS += ${VIDEO_IO_PATH}
    IDIRS += ${TIOVX_PATH}/conformance_tests

    STATIC_LIBS += $(IMAGING_LIBS)
    STATIC_LIBS += $(TEST_LIBS)

    CFLAGS      += -DBUILD_CT_TIOVX_VIDEO_IO

    ifeq ($(LDRA_COVERAGE_ENABLED_VIDEO_IO),yes)
        RUN_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS?=yes
        RUN_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS?=no
        RUN_CT_TIOVX_VIDEO_IO_DISPLAY_M2M_TESTS?=no
        RUN_CT_TIOVX_VIDEO_IO_CSITX_TESTS?=no
    else
        RUN_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS?=yes
        RUN_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS?=yes
        RUN_CT_TIOVX_VIDEO_IO_DISPLAY_M2M_TESTS?=yes
        ifeq ($(SOC),$(filter $(SOC), tda54))
            RUN_CT_TIOVX_VIDEO_IO_CSITX_TESTS?=no
        else
            RUN_CT_TIOVX_VIDEO_IO_CSITX_TESTS?=yes
        endif
    endif

    ifeq ($(SOC),$(filter $(SOC), am62a))
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS),yes)
            CFLAGS      += -DBUILD_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS
        endif
    endif

    ifeq ($(SOC),$(filter $(SOC), tda54))
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS),yes)
            CFLAGS      += -DBUILD_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS
        endif
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_CSITX_TESTS),yes)
            CFLAGS      += -DBUILD_CSITX
        endif
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS),yes)
            CFLAGS      += -DBUILD_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS
        endif
    endif

    ifeq ($(SOC),$(filter $(SOC), j721e j722s j721s2 j784s4 j742s2))
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS),yes)
            CFLAGS      += -DBUILD_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS
        endif
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_CSITX_TESTS),yes)
            CFLAGS      += -DBUILD_CSITX
        endif
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS),yes)
            CFLAGS      += -DBUILD_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS
        endif
    endif

    ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
        ifeq ($(RUN_CT_TIOVX_VIDEO_IO_DISPLAY_M2M_TESTS),yes)
            CFLAGS      += -DBUILD_DISPLAY_M2M
        endif
    endif

    ifeq ($(LDRA_COVERAGE_ENABLED_VIDEO_IO),yes)
        BUILD_DEFS += LDRA_COVERAGE_ENABLED_VIDEO_IO
    endif

    include $(FINALE)

endif

