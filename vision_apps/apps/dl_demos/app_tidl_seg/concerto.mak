ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

    include $(PRELUDE)

    TARGET      := vx_app_tidl_seg
    CPPSOURCES  := main.cpp
    CPPSOURCES  += config/app_seg_cfg_params.cpp

    CFLAGS += -Wno-unused-function -Wno-parentheses -Wno-sign-compare -Wno-format-truncation

    ifeq ($(TARGET_CPU),x86_64)

        TARGETTYPE  := exe

        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

        STATIC_LIBS += $(TIADALG_LIBS)

    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

            TARGETTYPE  := exe

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

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

    include $(FINALE)
endif
