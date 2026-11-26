ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A720 x86_64))

    include $(PRELUDE)

    TARGET      := vx_app_tidl_bev
    CPPSOURCES  := main.cpp bev_post_proc_module.cpp config/app_bev_cfg_params.cpp

    CFLAGS += -Wno-unused-function -Wno-parentheses -Wno-sign-compare -Wno-format-truncation


    ifeq ($(TARGET_CPU),x86_64)
        TARGETTYPE  := exe
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A720))
            TARGETTYPE  := exe

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

            IDIRS       += $(TIOVX_PATH)/source/include

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
    
    ifneq ($(SOC),$(filter $(SOC), j784s4 tda54))
        SKIPBUILD=1
    endif

    include $(FINALE)

endif
