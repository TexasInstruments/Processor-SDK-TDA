ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

    include $(PRELUDE)

    TARGET      := vx_app_test_framework
    TARGETTYPE  := exe
    CSOURCES    := $(call all-c-files)

    ifeq ($(TARGET_CPU),x86_64)
        CFLAGS      += -DTARGET_X86_64
        CFLAGS      += -DGL_ES -DSTANDALONE
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
        STATIC_LIBS += VXLIB_triangulatePoints_i32f_o32f_lib_x86_64
        SYS_SHARED_LIBS += X11
        # Not building for PC
        SKIPBUILD=1
    endif

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
        include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
        CFLAGS      += -DGL_ES
        ifeq ($(TARGET_OS), $(filter $(TARGET_OS), LINUX))
            CFLAGS      += -DEGL_NO_X11 -DPLATFORM_EGL
            SYS_SHARED_LIBS += gbm rt
            LDIRS           += $(LINUX_FS_PATH)/lib
        endif
        ifeq ($(TARGET_OS),QNX)
            SYS_SHARED_LIBS += screen
        endif
    endif

    IDIRS += $(VISION_APPS_KERNELS_IDIRS)
    IDIRS += $(VISION_APPS_APPLIBS_IDIRS)
    IDIRS += $(TEST_IDIRS)

    STATIC_LIBS += $(VISION_APPS_OPENGL_UTILS_LIBS)
    STATIC_LIBS += $(IMAGING_LIBS)
    STATIC_LIBS += $(VISION_APPS_SRV_LIBS)
    STATIC_LIBS += $(TEST_LIBS)

    SYS_SHARED_LIBS += EGL
    SYS_SHARED_LIBS += GLESv2

    ifeq ($(SOC),j722s)
        SKIPBUILD=1
    endif

    include $(FINALE)
endif
