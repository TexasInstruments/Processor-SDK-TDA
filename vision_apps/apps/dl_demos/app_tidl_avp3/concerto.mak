ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

include $(PRELUDE)

TARGET      := vx_app_tidl_avp3
CSOURCES    := main.c avp_scaler_module.c avp_dof_pyramid_module.c avp_dof_proc_module.c avp_pre_proc_module.c avp_tidl_module.c avp_post_proc_module.c fisheye_angle_table.c avp_draw_detections_module.c avp_dof_viz_module.c avp_img_mosaic_module.c avp_display_module.c

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-function
endif

ifeq ($(TARGET_CPU),x86_64)

TARGETTYPE  := exe

CSOURCES    += main_x86.c

include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak

IDIRS       += $(VISION_APPS_KERNELS_IDIRS)

STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)
STATIC_LIBS += $(TIADALG_LIBS)

endif

ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))

TARGETTYPE  := exe

CSOURCES    += main_linux_arm.c

include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak

IDIRS       += $(VISION_APPS_KERNELS_IDIRS)

STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)

endif
endif

IDIRS       += $(EDGEAI_IDIRS)
SHARED_LIBS += edgeai-apps-utils
SHARED_LIBS += edgeai-tiovx-kernels

ifeq ($(SOC),j722s)
SKIPBUILD=1
endif

include $(FINALE)

endif
