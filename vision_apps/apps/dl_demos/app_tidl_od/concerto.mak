ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)

    TARGET      := vx_app_tidl_od
    CSOURCES    := main.c
    CSOURCES    += app_pre_proc_module.c
    CSOURCES    += app_post_proc_module.c

    ifeq ($(HOST_COMPILER),GCC_LINUX)
        CFLAGS += -Wno-unused-function
    endif

    ifeq ($(TARGET_CPU),x86_64)

        TARGETTYPE  := exe
        CSOURCES    += main_x86.c

        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

        IDIRS       += $(APP_KERNELS_IDIRS)
        IDIRS       += $(VISION_APPS_MODULES_IDIRS)

        STATIC_LIBS += $(APP_KERNELS_LIBS)
        STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)
        STATIC_LIBS += $(TIADALG_LIBS)

    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))

            TARGETTYPE  := exe
            CSOURCES    += main_linux_arm.c

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

            IDIRS       += $(APP_KERNELS_IDIRS)
            IDIRS       += $(VISION_APPS_MODULES_IDIRS)

            STATIC_LIBS += $(APP_KERNELS_LIBS)
            STATIC_LIBS += $(APP_KERNELS_UTILS_LIBS)
            STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

            # AM62A Linux: Link drm_wrapper for DRM/KMS display with zero-copy
            ifeq ($(TARGET_PLATFORM)$(TARGET_OS), AM62ALINUX)
                STATIC_LIBS += app_utils_drm_wrapper
                SYS_SHARED_LIBS += drm
                IDIRS += $(LINUX_FS_PATH)/usr/include/libdrm/
                IDIRS += $(LINUX_FS_PATH)/usr/include/drm/
            endif

        endif
    endif

    ifeq ($(SOC), $(filter $(SOC), am62a))
        ifeq ($(TARGET_CPU),x86_64)
        SKIPBUILD=1
        endif
    endif

    include $(FINALE)
endif
