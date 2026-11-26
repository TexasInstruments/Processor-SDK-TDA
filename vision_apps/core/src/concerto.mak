ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 A720))

    include $(PRELUDE)
    TARGET      := vx_app_modules_core
    TARGETTYPE  := library

    CFLAGS      += -Wno-parentheses -Wno-sign-compare

    CPPSOURCES  := app_modules.cpp \
                   app_pipeline_config.cpp

    ifneq ($(SOC), $(filter $(SOC), tda54))
        CPPSOURCES  += app_camera_pipeline_setup.cpp
    endif

    IDIRS       += $(VISION_APPS_PATH)/core/include
    IDIRS       += $(VISION_APPS_PATH)/modules/include
    IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/include
    IDIRS       += $(APP_KERNELS_PATH)/kernels/img_proc/host
    IDIRS       += $(APP_KERNELS_PATH)/kernels/fileio/include
    IDIRS       += $(TIDL_PATH)/ti_dl/inc

    IDIRS       += $(TIADALG_PATH)/include
    IDIRS       += $(IVISION_PATH)
    IDIRS       += $(TIOVX_PATH)/include

    include $(FINALE)

endif
