ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)
    TARGET      := vx_app_modules
    TARGETTYPE  := library

        CSOURCES    := c/app_sensor_module.c
        CSOURCES    += c/app_capture_module.c
        CSOURCES    += c/app_viss_module.c
        CSOURCES    += c/app_ldc_module.c
        CSOURCES    += c/app_aewb_module.c
        CPPSOURCES  += app_aewb_module.cpp

        ifneq ($(SOC), $(filter $(SOC), tda54))
            CPPSOURCES  += app_sensor_module.cpp
            CPPSOURCES  += app_capture_module.cpp
            CPPSOURCES  += app_viss_module.cpp
            CPPSOURCES  += app_ldc_module.cpp
        endif

        ifneq ($(SOC), am62a)
            CSOURCES    += c/app_display_module.c
            CPPSOURCES  += app_display_module.cpp
        endif

        CSOURCES    += c/app_scaler_module.c
        CSOURCES    += c/app_tidl_module.c
        CSOURCES    += c/app_img_mosaic_module.c
        CSOURCES    += c/app_obj_arr_split_module.c

        CPPSOURCES  += app_scaler_module.cpp
        CPPSOURCES  += app_tidl_module.cpp
        CPPSOURCES  += app_img_mosaic_module.cpp
        CPPSOURCES  += app_pre_proc_module.cpp
        CPPSOURCES  += app_post_proc_module.cpp

        IDIRS       += $(IMAGING_IDIRS)
        IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
        IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/host
        IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/include
        IDIRS       += $(TIDL_PATH)/ti_dl/inc
        IDIRS       += $(VISION_APPS_PATH)/modules/include
        IDIRS       += $(VISION_APPS_PATH)/core/include
        IDIRS       += $(TIOVX_PATH)/include

        CFLAGS      += -Wno-error=parentheses -Wno-error=sign-compare                                                                                
        CPPFLAGS    += -Wno-error=parentheses -Wno-error=sign-compare

        include $(FINALE)

endif
