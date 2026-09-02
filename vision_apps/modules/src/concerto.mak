ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)
    TARGET      := vx_app_modules
    TARGETTYPE  := library

        ifneq ($(SOC), $(filter $(SOC), tda54))
            CSOURCES    := app_sensor_module.c
            CSOURCES    += app_capture_module.c
            CSOURCES    += app_viss_module.c
            CSOURCES    += app_ldc_module.c
        endif

        CSOURCES    += app_aewb_module.c

        ifneq ($(SOC), am62a)
            CSOURCES    += app_display_module.c
        endif

        CSOURCES    += app_scaler_module.c
        CSOURCES    += app_tidl_module.c
        CSOURCES    += app_img_mosaic_module.c
        CSOURCES    += app_obj_arr_split_module.c

        IDIRS       += $(IMAGING_IDIRS)
        IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
        IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/include
        IDIRS       += $(TIDL_PATH)/ti_dl/inc
        IDIRS       += $(VISION_APPS_PATH)/modules/include

        include $(FINALE)

endif
