ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)
    TARGET      := vx_app_tidl
    CSOURCES    := imagenet_class_labels.c main.c

    ifeq ($(TARGET_CPU),x86_64)
        TARGETTYPE  := exe
        CSOURCES    += main_x86.c
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
    endif

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
        ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
            TARGETTYPE  := exe
            CSOURCES    += main_linux_arm.c

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

            # AM62A Linux: Link drm_wrapper for DRM/KMS display with zero-copy
            ifeq ($(TARGET_PLATFORM)$(TARGET_OS), AM62ALINUX)
                STATIC_LIBS += app_utils_drm_wrapper
                SYS_SHARED_LIBS += drm
                IDIRS += $(LINUX_FS_PATH)/usr/include/libdrm/
                IDIRS += $(LINUX_FS_PATH)/usr/include/drm/
            endif
        endif
    endif

    ifeq ($(TARGET_OS),QNX)
        SHARED_LIBS += m
    endif

    ifeq ($(TARGET_OS),SYSBIOS)
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
            TARGETTYPE  := library
        endif
    endif

    include $(FINALE)
endif
