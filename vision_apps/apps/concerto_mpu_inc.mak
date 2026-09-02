ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))

    TEST_IDIRS =
    TEST_IDIRS += $(TIOVX_PATH)/conformance_tests

    IMAGING_IDIRS  =
    IMAGING_IDIRS += $(IMAGING_PATH)/kernels/include
    IMAGING_IDIRS += $(IMAGING_PATH)/sensor_drv/include
    IMAGING_IDIRS += $(IMAGING_PATH)/utils/itt_server/include
    IMAGING_IDIRS += $(IMAGING_PATH)/utils/network_api/include

    APP_KERNELS_IDIRS =
    APP_KERNELS_IDIRS += $(APP_KERNELS_PATH)/kernels
    APP_KERNELS_IDIRS += $(APP_KERNELS_PATH)/kernels/img_proc/include
    APP_KERNELS_IDIRS += $(APP_KERNELS_PATH)/kernels/img_proc/host
    APP_KERNELS_IDIRS += $(APP_KERNELS_PATH)/kernels/fileio/include

    VISION_APPS_MODULES_IDIRS =
    VISION_APPS_MODULES_IDIRS += $(VISION_APPS_PATH)/modules/include

    APP_SRV_IDIRS =
    APP_SRV_IDIRS += $(APP_KERNELS_PATH)/kernels/srv/include
    APP_SRV_IDIRS += $(APP_KERNELS_PATH)/kernels/srv/target/dsp
    APP_SRV_IDIRS += $(APP_KERNELS_PATH)/kernels/srv/target/gpu/3dsrv

    APP_SAMPLE_IDIRS += $(APP_KERNELS_PATH)/kernels/sample/include
    APP_SAMPLE_IDIRS += $(APP_KERNELS_PATH)/kernels/sample/host

    VISION_APPS_APPLIBS_IDIRS =
    VISION_APPS_APPLIBS_IDIRS += $(VISION_APPS_PATH)/applibs

    PTK_IDIRS =
    PTK_IDIRS += $(PTK_PATH)/include

    APP_STEREO_KERNELS_IDIRS =
    APP_STEREO_KERNELS_IDIRS += $(APP_KERNELS_PATH)/kernels/stereo/include

    PLATFORM_IDIRS =
    PLATFORM_IDIRS += $(PLATFORM_PATH)/hlos/include
    PLATFORM_IDIRS += $(PLATFORM_PATH)

    IDIRS += $(PLATFORM_IDIRS)

    # These rpath-link linker options are to provide directories for
    # secondary *.so file lookup
    ifeq ($(TARGET_OS),LINUX)
        $(_MODULE)_LOPT += -rpath-link=$(LINUX_FS_PATH)/usr/lib
        $(_MODULE)_LOPT += -rpath-link=$(LINUX_FS_PATH)/lib
        $(_MODULE)_LOPT += -rpath-link=$(LINUX_FS_PATH)/usr/lib/python3.8/site-packages/dlr
    endif
    ifeq ($(TARGET_OS),QNX)
        $(_MODULE)_LOPT += -rpath-link=$(QNX_TARGET)/usr/lib
        $(_MODULE)_LOPT += -rpath-link=$(QNX_TARGET)/lib
    endif

    CFLAGS+=-Wno-format-truncation

    ifeq ($(TARGET_OS), QNX)

        BUILD_PROFILE_QNX_SO = so.le
        BUILD_PROFILE_QNX_A = a.le
        ifneq ($(SOC), tda54)
            BUILD_PROFILE_QNX_SUFFIX =
        else
            BUILD_PROFILE_QNX_SUFFIX = S
        endif

        ifneq ($(SOC), tda54)
            LDIRS       += $(PSDK_QNX_PATH)/qnx/pdk_libs/pdk/aarch64/$(BUILD_PROFILE_QNX_SO)
            LDIRS       += $(PSDK_QNX_PATH)/qnx/pdk_libs/sciclient/aarch64/$(BUILD_PROFILE_QNX_SO)
            LDIRS       += $(PSDK_QNX_PATH)/qnx/pdk_libs/udmalld/aarch64/$(BUILD_PROFILE_QNX_SO)
            LDIRS       += $(PSDK_QNX_PATH)/qnx/sharedmemallocator/usr/aarch64/$(BUILD_PROFILE_QNX_SO)
            LDIRS       += $(PSDK_QNX_PATH)/qnx/resmgr/ipc_qnx_rsmgr/usr/aarch64/$(BUILD_PROFILE_QNX_SO)
            LDIRS       += $(PSDK_QNX_PATH)/qnx/resmgr/udma_qnx_rsmgr/usr/aarch64/$(BUILD_PROFILE_QNX_SO)

            SHARED_LIBS += sharedmemallocator$(BUILD_PROFILE_QNX_SUFFIX)
            SHARED_LIBS += tiipc-usr$(BUILD_PROFILE_QNX_SUFFIX)
            SHARED_LIBS += tiudma-usr$(BUILD_PROFILE_QNX_SUFFIX)
            SHARED_LIBS += ti-pdk$(BUILD_PROFILE_QNX_SUFFIX)
            SHARED_LIBS += ti-sciclient$(BUILD_PROFILE_QNX_SUFFIX)
            SHARED_LIBS += ti-udmalld$(BUILD_PROFILE_QNX_SUFFIX)
            ifeq ($(TARGET_PLATFORM), AM62A)
                SHARED_LIBS += screen c++ m
            endif
        else
            LDIRS       += $(PSDK_QNX_PATH)/output/stage/aarch64le/lib
            LDIRS       += $(PSDK_QNX_PATH)/output/stage/aarch64le/usr/lib
            STATIC_LIBS += sharedmemallocator$(BUILD_PROFILE_QNX_SUFFIX)
            STATIC_LIBS += tiipc-usr$(BUILD_PROFILE_QNX_SUFFIX)
            STATIC_LIBS += ti-dpl$(BUILD_PROFILE_QNX_SUFFIX)
            SHARED_LIBS += cache
        endif # ifneq ($(SOC), tda54)

    endif # ifeq ($(TARGET_OS), QNX)

    # Initialize the variables empty when building both debug and release binaries together
    TIOVX_LIBS  =
    IMAGING_LIBS =
    APP_SRV_LIBS  =
    APP_SAMPLE_LIBS  =
    APP_OPENGL_UTILS_LIBS =
    APP_KERNELS_LIBS  =
    APP_KERNELS_UTILS_LIBS =
    VISION_APPS_MODULES_LIBS  =
    TEST_LIBS =
    PTK_LIBS =
    APP_STEREO_LIBS =
    VISION_APPS_UTILS_LIBS  =
    HLOS_PLATFORM_LIBS  =

    # CTools include paths needed regardless of linking mode
    ifeq ($(CTOOLS_BUILD_ENABLED), yes)
        IDIRS += $(APP_UTILS_PATH)/utils/ctools/include
        IDIRS += $(APP_UTILS_PATH)/utils/ctools/include/soc/$(SOC)
        IDIRS += $(VISION_APPS_PATH)/utils/ctools_wrapper/include
    endif

    # This section is for apps to link against tivision_apps library instead of static libs
    ifeq ($(LINK_SHARED_OBJ)$(TARGETTYPE),yesexe)

        #$(info $(TARGET) links against libtivision_apps.so)
        SHARED_LIBS += tivision_apps

    # This section is for apps to link against static libs instead of tivision_apps library
    # Also used to create tivision_apps library (so we can maintain lib list in one place
    else   # ifeq ($(LINK_SHARED_OBJ),yes)

        LDIRS       += $(VISION_APPS_PATH)/out/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(PLATFORM_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(APP_KERNELS_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(APP_UTILS_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(VIDEO_IO_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(TIOVX_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(IMAGING_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(ETHFW_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(PTK_PATH)/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        LDIRS       += $(TIDL_PATH)/arm-tidl/tiovx_kernels/lib/$(TARGET_PLATFORM)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
        ifeq ($(TARGET_OS), LINUX)
            LDIRS       += $(LINUX_FS_PATH)/usr/lib
        endif
        ifeq ($(TARGET_OS), QNX)
            LDIRS       += $(QNX_HOST)/usr/lib
        endif

        TIOVX_LIBS += vx_vxu vx_framework
        TIOVX_LIBS += vx_kernels_host_utils vx_kernels_target_utils
        TIOVX_LIBS += vx_platform_board_hlos
        TIOVX_LIBS += vx_kernels_openvx_core
        TIOVX_LIBS += vx_kernels_openvx_ext vx_target_kernels_openvx_ext
        TIOVX_LIBS += vx_utils
        TIOVX_LIBS += vx_kernels_video_io
        TIOVX_LIBS += vx_kernels_hwa
        TIOVX_LIBS += vx_kernels_tidl
        TIOVX_LIBS += vx_kernels_tvm

        ifeq ($(ENABLE_NEW_TIDL_STRUCTURE),yes)
            # Include vx_nested_kernels_tidl for multicore devices only
            ifeq ($(SOC),$(filter $(SOC), j784s4 j722s j742s2))
                TIOVX_LIBS += vx_nested_kernels_tidl
            endif
        else
            TIOVX_LIBS += vx_nested_kernels_tidl
        endif

        ifneq ($(TARGET_PLATFORM), TDA54)
            TIOVX_LIBS += vx_tutorial
        endif

        ifeq ($(TARGET_PLATFORM)$(TARGET_OS), AM62AQNX)
            TIOVX_LIBS+=vx_target_kernels_capture
        endif

        IMAGING_LIBS += app_utils_iss

        ifeq ($(TARGET_PLATFORM), AM62A)
            ifeq ($(TISDK_IMAGE), adas)
                IMAGING_LIBS += vx_kernels_imaging
            endif
        else ifneq ($(TARGET_PLATFORM), AM62A)
            IMAGING_LIBS += vx_kernels_imaging
        endif

        ifneq ($(TARGET_PLATFORM), TDA54)
            ifeq ($(TARGET_OS), LINUX)
                IMAGING_LIBS += ti_imaging_aealg
                IMAGING_LIBS += ti_imaging_awbalg

                ifeq ($(SOC), am62a)
                    ifeq ($(TISDK_IMAGE), edgeai)
                        IMAGING_LIBS += ti_2a_wrapper
                    endif
                endif

                IMAGING_LIBS += ti_imaging_ittsrvr
                IMAGING_LIBS += app_utils_network_api
                IMAGING_LIBS += app_utils_itt_server
            endif
        endif

        ifeq ($(TARGET_PLATFORM)$(TARGET_OS), AM62AQNX)
            ifeq ($(TISDK_IMAGE), edgeai)
                IMAGING_LIBS = vx_target_kernels_imaging_aewb
                IMAGING_LIBS += ti_2a_wrapper
                IMAGING_LIBS += ti_imaging_aealg
                IMAGING_LIBS += ti_imaging_awbalg
                IMAGING_LIBS += ti_imaging_dcc
                IMAGING_LIBS += ti_imaging_ittsrvr
                IMAGING_LIBS += ti_imaging_sensordrv
            endif
        endif

        VISION_APPS_UTILS_LIBS += app_utils_mem
        VISION_APPS_UTILS_LIBS += app_utils_ipc
        VISION_APPS_UTILS_LIBS += app_utils_console_io
        VISION_APPS_UTILS_LIBS += app_utils_timer
        VISION_APPS_UTILS_LIBS += app_utils_file_io
        VISION_APPS_UTILS_LIBS += app_utils_remote_service
        VISION_APPS_UTILS_LIBS += app_utils_perf_stats
        ifeq ($(CTOOLS_BUILD_ENABLED), yes)
            VISION_APPS_UTILS_LIBS += app_utils_ctools
            VISION_APPS_UTILS_LIBS += app_utils_ctools_wrapper
        endif

        ifneq ($(TARGET_PLATFORM), AM62A)
            VISION_APPS_UTILS_LIBS += app_utils_grpx
            VISION_APPS_UTILS_LIBS += app_utils_draw2d
        endif

        ifeq ($(TARGET_PLATFORM)$(TARGET_OS), AM62AQNX)
            ifeq ($(TISDK_IMAGE), edgeai)
                VISION_APPS_UTILS_LIBS += app_utils_udma
                VISION_APPS_UTILS_LIBS += app_utils_sensors
                VISION_APPS_UTILS_LIBS += app_utils_iss
                VISION_APPS_UTILS_LIBS += app_utils_grpx
                VISION_APPS_UTILS_LIBS += app_utils_draw2d
            endif
        endif

        ifeq ($(TARGET_PLATFORM), AM62A)
            ifeq ($(TISDK_IMAGE), adas)
                VISION_APPS_UTILS_LIBS += app_utils_grpx
                VISION_APPS_UTILS_LIBS += app_utils_draw2d
            endif
        endif

        ifneq ($(TARGET_PLATFORM), TDA54)
            VISION_APPS_UTILS_LIBS += app_utils_hwa
        endif
        HLOS_PLATFORM_LIBS += app_init_hlos_common
        HLOS_PLATFORM_LIBS += ipc_common

        APP_OPENGL_UTILS_LIBS += app_utils_opengl
        APP_KERNELS_UTILS_LIBS += vx_kernel_utils

        APP_SAMPLE_LIBS  += vx_kernels_sample vx_target_kernels_sample_arm

        APP_SRV_LIBS  += vx_kernels_srv vx_target_kernels_srv_gpu
        APP_SRV_LIBS  += vx_applib_srv_bowl_lut_gen
        APP_SRV_LIBS  += vx_applib_srv_calibration
        APP_SRV_LIBS  += vx_srv_render_utils
        APP_SRV_LIBS  += vx_srv_render_utils_tools

        APP_KERNELS_LIBS += vx_kernels_img_proc
        APP_KERNELS_LIBS += vx_target_kernels_img_proc_arm
        APP_KERNELS_LIBS += vx_kernels_fileio
        APP_KERNELS_LIBS += vx_target_kernels_fileio

        ifneq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), AM62A))
            APP_STEREO_LIBS += vx_kernels_common
            APP_STEREO_LIBS += vx_kernels_stereo
            APP_STEREO_LIBS += vx_target_kernels_stereo
        endif

        # vx_app_modules not used for AM62A EdgeAI Linux
        ifneq ($(TARGET_PLATFORM),AM62A)
            VISION_APPS_MODULES_LIBS += vx_app_modules
        else ifneq ($(TISDK_IMAGE),edgeai)
            VISION_APPS_MODULES_LIBS += vx_app_modules
        else ifneq ($(TARGET_OS),LINUX)
            VISION_APPS_MODULES_LIBS += vx_app_modules
        else
        endif

        PTK_LIBS += ptk_base
        PTK_LIBS += ptk_algos

        TEST_LIBS += vx_tiovx_tests vx_tiovx_internal_tests vx_conformance_tests vx_conformance_engine vx_conformance_tests_testmodule
        TEST_LIBS += vx_kernels_openvx_ext_tests
        TEST_LIBS += vx_kernels_test_kernels_tests vx_kernels_test_kernels
        TEST_LIBS += vx_target_kernels_source_sink vx_kernels_hwa_tests
        TEST_LIBS += vx_kernels_video_io_tests
        TEST_LIBS += vx_tiovx_tidl_tests
        ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721E J722S J721S2 J784S4 J742S2))
            TEST_LIBS += vx_target_kernels_vpac_aewb
        endif

        ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721E J721S2 J784S4 J742S2 TDA54))
            ifneq ($(TARGET_PLATFORM), TDA54)
                TEST_LIBS += vx_tiovx_tvm_tests
                TEST_LIBS += vx_kernels_srv_tests
                TEST_LIBS += vx_applib_tests
            endif
        endif

        STATIC_LIBS += $(TIOVX_LIBS)
        STATIC_LIBS += $(VISION_APPS_UTILS_LIBS)
        STATIC_LIBS += $(HLOS_PLATFORM_LIBS)
        ifeq ($(TARGET_OS),QNX)
            STATIC_LIBS += app_utils_rtos
        endif

        ifeq ($(TARGET_OS),LINUX)
            SYS_SHARED_LIBS += stdc++ m rt pthread ti_rpmsg_char
        endif
        ifeq ($(TARGET_OS),QNX)
            SHARED_LIBS += c++ m
            ifeq ($(QNX_SDP_VERSION),710)
                SHARED_LIBS += c++fs
            endif
        endif

    endif  # ifeq ($(LINK_SHARED_OBJ),yes)

endif  # ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53 A720))
