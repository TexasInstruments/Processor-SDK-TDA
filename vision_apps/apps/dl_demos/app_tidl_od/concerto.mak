ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)
    
    TARGET      := vx_app_tidl_od
    CPPSOURCES  := main.cpp
    CPPSOURCES  +=config/app_od_cfg_params.cpp

    CFLAGS += -Wno-unused-function -Wno-parentheses -Wno-sign-compare -Wno-format-truncation

    ifeq ($(TARGET_CPU),x86_64)

        TARGETTYPE  := exe

        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

        STATIC_LIBS += $(TIADALG_LIBS)

    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))

            TARGETTYPE  := exe

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

 	IDIRS       += $(APP_KERNELS_IDIRS)
    IDIRS       += $(VISION_APPS_MODULES_IDIRS)

	STATIC_LIBS += $(IMAGING_LIBS)
	STATIC_LIBS += $(APP_KERNELS_LIBS)
	STATIC_LIBS += $(APP_KERNELS_UTILS_LIBS)
    STATIC_LIBS += $(VISION_APPS_MODULES_LIBS)

    ifeq ($(TARGET_OS),QNX)
        SHARED_LIBS += c++ m
    endif

    ifeq ($(SOC), $(filter $(SOC), am62a))
        ifeq ($(TARGET_CPU),x86_64)
        SKIPBUILD=1
        endif
    endif

    include $(FINALE)
endif
