# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)
    TEST_IDIRS =
    TEST_IDIRS += $(TIOVX_PATH)/conformance_tests

    IMAGING_IDIRS  =
    IMAGING_IDIRS += $(IMAGING_PATH)/kernels/include
    IMAGING_IDIRS += $(IMAGING_PATH)/sensor_drv/include

    VISION_APPS_KERNELS_IDIRS =
    VISION_APPS_KERNELS_IDIRS += $(VISION_APPS_PATH)/kernels
    VISION_APPS_KERNELS_IDIRS += $(VISION_APPS_PATH)/kernels/img_proc/include
    VISION_APPS_KERNELS_IDIRS += $(VISION_APPS_PATH)/kernels/fileio/include

    VISION_APPS_MODULES_IDIRS =
    VISION_APPS_MODULES_IDIRS += $(VISION_APPS_PATH)/modules/include

    VISION_APPS_SRV_IDIRS =
    VISION_APPS_SRV_IDIRS += $(VISION_APPS_PATH)/kernels/srv/include
    VISION_APPS_SRV_IDIRS += $(VISION_APPS_PATH)/kernels/srv/c66
    VISION_APPS_SRV_IDIRS += $(VISION_APPS_PATH)/kernels/srv/gpu/3dsrv

    VISION_APPS_SAMPLE_IDIRS =
    VISION_APPS_SAMPLE_IDIRS += $(VISION_APPS_PATH)/kernels/sample/include
    VISION_APPS_SAMPLE_IDIRS += $(VISION_APPS_PATH)/kernels/sample/host

    VISION_APPS_APPLIBS_IDIRS =
    VISION_APPS_APPLIBS_IDIRS += $(VISION_APPS_PATH)/applibs

    PTK_IDIRS =
    PTK_IDIRS += $(PTK_PATH)/include

    VISION_APPS_STEREO_KERNELS_IDIRS =
    VISION_APPS_STEREO_KERNELS_IDIRS += $(VISION_APPS_PATH)/kernels/stereo/include

    EDGEAI_IDIRS =
    EDGEAI_IDIRS += $(EDGEAI_UTILS_PATH)/include
    EDGEAI_IDIRS += $(EDGEAI_KERNELS_PATH)/include

    BUILD_PROFILE_EDGEAI_REL = Release

    LDIRS       += $(VISION_APPS_PATH)/out/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    LDIRS       += $(APP_UTILS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    LDIRS       += $(TIOVX_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    LDIRS       += $(IMAGING_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    LDIRS       += $(TIADALG_PATH)/lib/$(TARGET_CPU)/$(TARGET_BUILD)
    LDIRS       += $(CGT7X_ROOT)/host_emulation
    LDIRS       += $(MMALIB_PATH)/lib/$(C7X_VERSION)/$(TARGET_BUILD)
    LDIRS       += $(PTK_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    LDIRS       += $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
        LDIRS       += $(TIDL_PATH)/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
    endif

    LDIRS       += $(TIDL_PATH)/ti_dl/lib/$(TARGET_SOC)/$(TARGET_PLATFORM)/algo/$(TARGET_BUILD)
    LDIRS       += $(EDGEAI_UTILS_PATH)/HOST/lib/$(BUILD_PROFILE_EDGEAI_REL)
    LDIRS       += $(EDGEAI_KERNELS_PATH)/HOST/lib/$(BUILD_PROFILE_EDGEAI_REL)

    TIOVX_LIBS  =
    IMAGING_LIBS =
    VISION_APPS_SRV_LIBS  =
    VISION_APPS_SAMPLE_LIBS  =
    VISION_APPS_OPENGL_UTILS_LIBS =
    VISION_APPS_KERNELS_LIBS  =
    VISION_APPS_MODULES_LIBS  =
    TEST_LIBS =
    PTK_LIBS =
    VISION_APPS_STEREO_LIBS =
    VISION_APPS_UTILS_LIBS  =

    # This section is for apps to link against tivision_apps library instead of static libs
    ifeq ($(LINK_SHARED_OBJ)$(TARGETTYPE),yesexe)
        #$(info $(TARGET) links against libtivision_apps.so)
        SHARED_LIBS += tivision_apps
    # This section is for apps to link against static libs instead of tivision_apps library
    # Also used to create tivision_apps library (so we can maintain lib list in one place
    else   # ifeq ($(LINK_SHARED_OBJ),yes)
        ifeq ($(RTOS_SDK), mcu_plus_sdk)
            LDIRS+= $(MCU_PLUS_SDK_PATH)/source/drivers/dmautils/lib/
        else ifeq ($(RTOS_SDK), mcu_sdk)
            ifeq ($(TARGET_BUILD),debug)
                LDIRS += $(MCU_SDK_PATH)/build/tda54/lib/Debug
            else
                LDIRS += $(MCU_SDK_PATH)/build/tda54/lib/Release
            endif
            LDIRS += $(DMA_UTILS_PATH)/lib
        else
            LDIRS+= $(PDK_PATH)/packages/ti/drv/udma/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
            LDIRS+= $(PDK_PATH)/packages/ti/csl/lib/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
            LDIRS+= $(PDK_PATH)/packages/ti/drv/sciclient/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
            LDIRS+= $(PDK_PATH)/packages/ti/osal/lib/nonos/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
        endif

        CFLAGS += -Wno-unused-but-set-variable
        CFLAGS += -Wno-unused-variable
        CFLAGS += -Wno-unused-result
        CFLAGS += -Wno-maybe-uninitialized

        APP_UTILS_LIBS =
        APP_UTILS_LIBS += app_utils_misc
        APP_UTILS_LIBS += app_utils_mem
        APP_UTILS_LIBS += app_utils_perf_stats
        APP_UTILS_LIBS += app_utils_console_io
        APP_UTILS_LIBS += app_utils_file_io
        ifeq ($(SOC),$(filter $(SOC), tda54))
            APP_UTILS_LIBS += app_utils_ipc
            APP_UTILS_LIBS += app_utils_timer
            APP_UTILS_LIBS += app_utils_pc_osal
            APP_UTILS_LIBS += app_utils_remote_service
        endif

        SYS_STATIC_LIBS += $(APP_UTILS_LIBS)

        TIOVX_LIBS  =
        TIOVX_LIBS += vx_vxu vx_framework
        TIOVX_LIBS += vx_kernels_host_utils vx_kernels_target_utils
        TIOVX_LIBS += vx_platform_pc
        TIOVX_LIBS += vx_kernels_openvx_core vx_target_kernels_openvx_core
        TIOVX_LIBS += vx_kernels_openvx_ext vx_target_kernels_openvx_ext
        TIOVX_LIBS += vx_target_kernels_tutorial
        ifneq ($(SOC), tda54)
            TIOVX_LIBS += vx_app_c7x_target_kernel
        endif
        TIOVX_LIBS += vx_utils
        TIOVX_LIBS += vx_kernels_tidl

        ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
            # Include vx_nested_kernels_tidl for multicore devices only
            ifeq ($(SOC),$(filter $(SOC), j784s4 j722s j742s2))
                TIOVX_LIBS += vx_nested_kernels_tidl
            endif
        else
            TIOVX_LIBS += vx_nested_kernels_tidl
        endif

        TIOVX_LIBS += vx_target_kernels_tidl vx_target_kernels_ivision_common

        IMAGING_LIBS =
        IMAGING_LIBS += vx_kernels_imaging
        IMAGING_LIBS += ti_imaging_aealg
        IMAGING_LIBS += ti_imaging_awbalg
        ifeq ($(SOC),$(filter $(SOC), j721e j722s j721s2 j784s4 j742s2))
            IMAGING_LIBS += vx_target_kernels_imaging_aewb
        endif

        ifeq ($(SOC), am62a)
            ifeq ($(TISDK_IMAGE), adas)
                IMAGING_LIBS += vx_target_kernels_imaging_aewb
            endif
        endif

        TIADALG_LIBS  =
        ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
            TIADALG_LIBS += tiadalg_fisheye_transformation
            TIADALG_LIBS += tiadalg_image_preprocessing
            TIADALG_LIBS += tiadalg_dof_plane_seperation
            TIADALG_LIBS += tiadalg_visual_localization
            TIADALG_LIBS += tiadalg_select_top_feature
            TIADALG_LIBS += tiadalg_solve_pnp
            TIADALG_LIBS += tiadalg_sparse_upsampling
            TIADALG_LIBS += tiadalg_image_color_blending
            TIADALG_LIBS += tiadalg_image_recursive_nms
            TIADALG_LIBS += tiadalg_structure_from_motion
            ifeq ($(SOC),j721e)
                TIADALG_LIBS += c6xsim
            endif
        endif

        VISION_APPS_UTILS_LIBS  =
        ifneq ($(SOC),$(filter $(SOC), am62a))
            VISION_APPS_UTILS_LIBS += app_utils_draw2d
        else ifeq ($(TISDK_IMAGE), adas)
            VISION_APPS_UTILS_LIBS += app_utils_draw2d
        endif
        VISION_APPS_UTILS_LIBS += app_utils_init
        ifeq ($(SOC),$(filter $(SOC), tda54))
            VISION_APPS_UTILS_LIBS += app_utils_init_vdk
        endif
        ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
            VISION_APPS_UTILS_LIBS += app_utils_grpx
            VISION_APPS_UTILS_LIBS += app_utils_hwa
        endif

        VISION_APPS_SRV_LIBS  =
        VISION_APPS_SRV_LIBS  += vx_kernels_srv vx_target_kernels_srv_gpu
        VISION_APPS_SRV_LIBS  += vx_target_kernels_srv_c66
        VISION_APPS_SRV_LIBS  += vx_applib_srv_bowl_lut_gen
        VISION_APPS_SRV_LIBS  += vx_applib_srv_calibration
        VISION_APPS_SRV_LIBS  += vx_srv_render_utils

        VISION_APPS_SAMPLE_LIBS  =
        VISION_APPS_SAMPLE_LIBS  += vx_kernels_sample vx_target_kernels_sample_a72

        VISION_APPS_OPENGL_UTILS_LIBS =
        VISION_APPS_OPENGL_UTILS_LIBS += app_utils_opengl

        VISION_APPS_KERNELS_LIBS  =
        VISION_APPS_KERNELS_LIBS += vx_kernels_img_proc
        VISION_APPS_KERNELS_LIBS += vx_target_kernels_img_proc_a72
        VISION_APPS_KERNELS_LIBS += vx_kernels_fileio
        VISION_APPS_KERNELS_LIBS += vx_target_kernels_fileio
        ifneq ($(SOC), am62a)
            VISION_APPS_KERNELS_LIBS += vx_target_kernels_img_proc_r5f
        endif
        ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
            VISION_APPS_KERNELS_LIBS += vx_target_kernels_img_proc_c66
            VISION_APPS_KERNELS_LIBS += vx_target_kernels_img_proc_c71
        endif

        VISION_APPS_MODULES_LIBS  =
        VISION_APPS_MODULES_LIBS += vx_app_modules

        PTK_LIBS =
        PTK_LIBS += ptk_base
        PTK_LIBS += ptk_algos

        VISION_APPS_STEREO_LIBS =
        VISION_APPS_STEREO_LIBS += vx_kernels_common
        VISION_APPS_STEREO_LIBS += vx_kernels_stereo
        VISION_APPS_STEREO_LIBS += vx_target_kernels_stereo

        TEST_LIBS =
        TEST_LIBS += vx_tiovx_tests vx_tiovx_internal_tests vx_conformance_tests vx_conformance_tests_testmodule
        ifeq ($(SOC),$(filter $(SOC), j721e j722s j721s2 j784s4 j742s2))
            TEST_LIBS += vx_target_kernels_vpac_aewb
        endif
        TEST_LIBS += vx_kernels_openvx_ext_tests
        TEST_LIBS += vx_tiovx_tidl_tests
        ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
            TEST_LIBS += vx_kernels_srv_tests
            TEST_LIBS += vx_applib_tests
        endif

        MMA_LIBS =
        MMA_LIBS += mmalib_cn_x86_64
        MMA_LIBS += mmalib_x86_64
        MMA_LIBS += common_x86_64

        TIDL_LIBS =
        TIDL_LIBS += tidl_algo
        TIDL_LIBS += tidl_custom
        TIDL_LIBS += tidl_avx_kernels
        ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
            TIDL_LIBS += tidl_priv
            TIDL_LIBS += tidl_kernels
            TIDL_LIBS += tidl_ref
        else
            TIDL_LIBS += tidl_priv_algo
            TIDL_LIBS += tidl_obj_algo
        endif

        PDK_LIBS =
        ifeq ($(SOC),$(filter $(SOC), j721e j721s2 j784s4 j742s2))
            PDK_LIBS += dmautils.lib
            PDK_LIBS += udma.lib
            PDK_LIBS += sciclient.lib
            PDK_LIBS += ti.csl.lib
            PDK_LIBS += ti.csl.init.lib
            PDK_LIBS += ti.osal.lib
        endif

        MCU_PLUS_SDK_LIBS =
        MCU_SDK_LIBS =

        ifeq ($(SOC), am62a)
            MCU_PLUS_SDK_LIBS += dmautils.am62ax.c75x.ti-c7x-hostemu.$(TARGET_BUILD).lib
        endif
        ifeq ($(SOC), j722s)
            MCU_PLUS_SDK_LIBS += dmautils.j722s.c75ssx-0.ti-c7x-hostemu.$(TARGET_BUILD).lib
        endif
        ifeq ($(SOC), tda54)
            MCU_SDK_LIBS += dmautils.tda54.c76x.ti-c7x-hostemu.$(TARGET_BUILD).lib
            MCU_SDK_LIBS += libarch-ti_sdk_cfg_default_hostemu_gcc-linux.a
            MCU_SDK_LIBS += libdrivers-ti_sdk_cfg_default_hostemu_gcc-linux.a
            MCU_SDK_LIBS += libhal-ti_sdk_cfg_default_hostemu_gcc-linux.a
            MCU_SDK_LIBS += libtda54_hostemu_gcc-linux.a
            MCU_SDK_LIBS += libti_sdk_cfg_default_hostemu_gcc-linux.a
            MCU_SDK_LIBS += libutils_nortos-ti_sdk_cfg_default_hostemu_gcc-linux.a
        endif

        ADDITIONAL_STATIC_LIBS += $(PDK_LIBS)
        ADDITIONAL_STATIC_LIBS += $(MCU_PLUS_SDK_LIBS)
        ADDITIONAL_STATIC_LIBS += $(MCU_SDK_LIBS)

        STATIC_LIBS = $(MMA_LIBS)
        STATIC_LIBS += $(TIOVX_LIBS)
        STATIC_LIBS += $(TIDL_LIBS)
        STATIC_LIBS += vxlib_$(TARGET_CPU) c6xsim_$(TARGET_CPU)_C66
        STATIC_LIBS += $(VISION_APPS_UTILS_LIBS)
        STATIC_LIBS += $(VISION_APPS_KERNELS_LIBS)

        ifeq ($(SOC), j722s)
            STATIC_LIBS += C7524-MMA2_256-host-emulation
        else ifeq ($(SOC), tda54)
            STATIC_LIBS += $(C7X_VERSION)-MMA3_1024-host-emulation
        else
            STATIC_LIBS += $(C7X_VERSION)-host-emulation
        endif

        SYS_SHARED_LIBS += dl png z stdc++ m rt

        include $(TIOVX_PATH)/conformance_tests/kernels/concerto_inc.mak
        include $(IMAGING_PATH)/build_flags.mak
        include $(IMAGING_PATH)/kernels/concerto_inc.mak
        include $(VIDEO_IO_PATH)/build_flags.mak
        include $(VIDEO_IO_PATH)/kernels/concerto_inc.mak

    endif  # ifeq ($(LINK_SHARED_OBJ),yes)

endif
