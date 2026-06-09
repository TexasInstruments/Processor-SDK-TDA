ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), x86_64 A72 A53))

    include $(PRELUDE)

    TARGET      := vx_app_conformance
    TARGETTYPE  := exe
    CSOURCES    := $(call all-c-files)

    ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
        include $(VISION_APPS_PATH)/apps/concerto_mpu_inc.mak
    endif

    ifeq ($(TARGET_CPU),x86_64)
        include $(VISION_APPS_PATH)/apps/concerto_x86_64_inc.mak
    else
        CFLAGS += -DMPU
    endif

    IDIRS += ${TIOVX_PATH}
    IDIRS += ${TIOVX_PATH}/conformance_tests
    IDIRS += $(TIDL_PATH)
    IDIRS += $(TIDL_PATH)/ti_dl
    IDIRS += $(TIDL_PATH)/arm-tidl

    STATIC_LIBS += $(IMAGING_LIBS)
    STATIC_LIBS += $(TEST_LIBS)

    ifeq ($(BUILD_CT_TIOVX),yes)
        CFLAGS += -DBUILD_CT_TIOVX
    endif
    ifeq ($(BUILD_CT_TIOVX_INTERNAL),yes)
        CFLAGS += -DBUILD_CT_TIOVX_INTERNAL
    endif
    ifeq ($(BUILD_CT_KHR),yes)
        CFLAGS += -DBUILD_CT_KHR
        CFLAGS += -DOPENVX_USE_USER_DATA_OBJECT
    endif
    ifeq ($(BUILD_CT_TIOVX_HWA),yes)
        CFLAGS += -DBUILD_CT_TIOVX_HWA
    endif
    ifeq ($(BUILD_TYPE),dev)
        CFLAGS += -DBUILD_DEV
    endif
    ifeq ($(BUILD_CORE_KERNELS),yes)
        CFLAGS += -DBUILD_CORE_KERNELS
    endif
    ifeq ($(BUILD_EXT_KERNELS),yes)
        CFLAGS += -DBUILD_EXT_KERNELS
    endif
    ifeq ($(BUILD_TEST_KERNELS),yes)
        CFLAGS += -DBUILD_TEST_KERNELS
    endif

    include $(FINALE)

endif

