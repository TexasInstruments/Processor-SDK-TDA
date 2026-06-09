ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A720 x86_64))

    include $(PRELUDE)

    TARGET      := vx_app_tidl_bev
    CSOURCES    := main.c bev_scaler_module.c bev_pre_proc_module_batch.c bev_tidl_module.c bev_img_mosaic_module.c bev_post_proc_module.c  bev_display_module.c

    ifeq ($(HOST_COMPILER),GCC_LINUX)
        CFLAGS += -Wno-unused-function
    endif

    ifeq ($(TARGET_CPU),x86_64)
        TARGETTYPE  := exe
        CSOURCES    += main_x86.c
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
    endif

    ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A720))
            TARGETTYPE  := exe
            CSOURCES    += main_linux_arm.c

            include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

            IDIRS       += $(EDGEAI_KERNELS_PATH)/include
            IDIRS       += $(TIOVX_PATH)/source/include

            SHARED_LIBS += edgeai-apps-utils
            SHARED_LIBS += edgeai-tiovx-kernels
        endif
    endif

    IDIRS       += $(EDGEAI_IDIRS)
    IDIRS       += $(VISION_APPS_KERNELS_IDIRS)

    STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)

    SHARED_LIBS += edgeai-apps-utils
    SHARED_LIBS += edgeai-tiovx-kernels

    ifneq ($(SOC),$(filter $(SOC), j784s4 tda54))
        SKIPBUILD=1
    endif

    include $(FINALE)

endif
